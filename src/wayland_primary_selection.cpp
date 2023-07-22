#include "wayland_primary_selection.h"

#include <QtWaylandClient/private/qwaylandnativeinterface_p.h>
#include <unistd.h>

#include <QDebug>
#include <QGuiApplication>

void registryHandleGlobal(void *data,
                          struct wl_registry *wl_registry,
                          uint32_t name,
                          const char *interface,
                          uint32_t version);

void deviceHandleDataOffer(
    void *data,
    struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
    struct zwp_primary_selection_offer_v1 *offer);

void deviceHandleSelection(
    void *data,
    struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
    struct zwp_primary_selection_offer_v1 *offer);

void offerHandle(
    void *data,
    struct zwp_primary_selection_offer_v1 *zwp_primary_selection_offer_v1,
    const char *mime_type);

WaylandPrimarySelection::WaylandPrimarySelection(QObject *parent)
    : QObject(parent) {
    display_ = static_cast<wl_display *>(
        QGuiApplication::platformNativeInterface()
            ->nativeResourceForIntegration("wl_display"));
    if (!display_) {
        qFatal("Failed to connect to Wayland display");
        abort();
    }
    registry_ = wl_display_get_registry(display_);
    seat_ = nullptr;
    device_manager_ = nullptr;

    registry_listener_ = {.global = registryHandleGlobal,
                          .global_remove = [](void *data,
                                              struct wl_registry *wl_registry,
                                              uint32_t name) {
                              qDebug() << tr("global remove: %1").arg(name);
                          }};

    wl_registry_add_listener(registry_, &registry_listener_, this);

    // init device_ when seat_ and device_manager_ is ready
    connect(this, &WaylandPrimarySelection::seatReady, [this]() {
        if (device_manager_) {
            device_ = zwp_primary_selection_device_manager_v1_get_device(
                device_manager_,
                seat_);
            emit deviceReady();
        }
    });

    // init device_ when seat_ and device_manager_ is ready
    connect(this, &WaylandPrimarySelection::deviceManagerReady, [this]() {
        if (seat_) {
            device_ = zwp_primary_selection_device_manager_v1_get_device(
                device_manager_,
                seat_);
            emit deviceReady();
        }
    });

    // get primary selection when window forcus
    offer_listener_ = {.offer = offerHandle};
    device_listener_ = {.data_offer = deviceHandleDataOffer,
                        .selection = deviceHandleSelection};
    connect(this, &WaylandPrimarySelection::deviceReady, [this]() {
        zwp_primary_selection_device_v1_add_listener(device_,
                                                     &device_listener_,
                                                     this);
    });

    wl_display_roundtrip(display_);
}

WaylandPrimarySelection::~WaylandPrimarySelection() {
    // wl_registry_destroy(registry_);
    // wl_display_disconnect(display_);
}

WaylandPrimarySelection *WaylandPrimarySelection::instance() {
    static WaylandPrimarySelection instance;
    return &instance;
}

// QMimeData WaylandPrimarySelection::mimeData() const { return mime_date_; }

QString WaylandPrimarySelection::text() const { return selection_; }

void registryHandleGlobal(void *data,
                          struct wl_registry *wl_registry,
                          uint32_t name,
                          const char *interface,
                          uint32_t version) {
    // bind zwp_primary_selection_device_manager_v1
    if (!strcmp(interface,
                ::zwp_primary_selection_device_manager_v1_interface.name)) {
        auto *primary_selection = static_cast<WaylandPrimarySelection *>(data);
        primary_selection->initDeviceManager(wl_registry, name, version);
        emit primary_selection->deviceManagerReady();
        // qDebug() << QObject::tr("device manager ready");
    } else if (!strcmp(interface, ::wl_seat_interface.name)) {
        auto *primary_selection = static_cast<WaylandPrimarySelection *>(data);
        primary_selection->initSeat(wl_registry, name, version);
        emit primary_selection->seatReady();
    }
}

void deviceHandleDataOffer(
    void *data,
    struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
    struct zwp_primary_selection_offer_v1 *offer) {
    auto *primary_selection = static_cast<WaylandPrimarySelection *>(data);
    zwp_primary_selection_offer_v1_add_listener(
        offer,
        &(primary_selection->offer_listener_),
        data);
}

void deviceHandleSelection(
    void *data,
    struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
    struct zwp_primary_selection_offer_v1 *offer) {
    if (!offer) {
        qWarning() << QObject::tr(
            "zwp_primary_selection_offer_v1 *offer is empty");
        return;
    }

    // Get data from pipe
    int fds[2];
    if (pipe(fds) == -1) {
        qWarning() << QObject::tr("pipe error");
        return;
    }
    zwp_primary_selection_offer_v1_receive(offer, "text/plain", fds[1]);
    close(fds[1]);

    auto *primary_selection = static_cast<WaylandPrimarySelection *>(data);

    // Make sure request is sent to the compositor before blocking read
    wl_display_roundtrip(primary_selection->display_);

    primary_selection->selection_.clear();
    while (true) {
        char buf[1024];
        ssize_t n = read(fds[0], buf, sizeof(buf) - 1);
        if (n <= 0) {
            break;
        }
        buf[n] = '\0';
        primary_selection->selection_.append(buf);
    }

    close(fds[0]);
    zwp_primary_selection_offer_v1_destroy(offer);
}

void offerHandle(
    void *data,
    struct zwp_primary_selection_offer_v1 *zwp_primary_selection_offer_v1,
    const char *mime_type) {
    qDebug() << QObject::tr("MIME TYPE: %1").arg(mime_type);
    // TODO
}