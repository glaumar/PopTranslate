#ifndef WAYLAND_PRIMARY_SELECTION_H
#define WAYLAND_PRIMARY_SELECTION_H

#include <QtWaylandClient/private/wayland-wp-primary-selection-unstable-v1-client-protocol.h>
#include <wayland-client.h>

#include <QMimeData>
#include <QObject>

class WaylandPrimarySelection : public QObject {
    Q_OBJECT
   public:
    static WaylandPrimarySelection *instance();
    ~WaylandPrimarySelection();
    // QMimeData mimeData() const;
    QString text() const;

    WaylandPrimarySelection(WaylandPrimarySelection const &) = delete;
    void operator=(WaylandPrimarySelection const &) = delete;
    WaylandPrimarySelection(WaylandPrimarySelection &&) = delete;
    void operator=(WaylandPrimarySelection &&) = delete;

    // callbakcs
    friend void registryHandleGlobal(void *data,
                                     struct wl_registry *wl_registry,
                                     uint32_t name,
                                     const char *interface,
                                     uint32_t version);
    friend void deviceHandleDataOffer(
        void *data,
        struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
        struct zwp_primary_selection_offer_v1 *offer);
    friend void deviceHandleSelection(
        void *data,
        struct zwp_primary_selection_device_v1 *zwp_primary_selection_device_v1,
        struct zwp_primary_selection_offer_v1 *offer);
    friend void offerHandle(
        void *data,
        struct zwp_primary_selection_offer_v1 *zwp_primary_selection_offer_v1,
        const char *mime_type);

   signals:
    // void selectionChanged();
    void deviceManagerReady();
    void seatReady();
    void deviceReady();

   private:
    explicit WaylandPrimarySelection(QObject *parent = nullptr);
    void initDeviceManager(struct ::wl_registry *registry,
                           int id,
                           int version) {
        device_manager_ =
            static_cast<zwp_primary_selection_device_manager_v1 *>(
                wl_registry_bind(
                    registry,
                    id,
                    &zwp_primary_selection_device_manager_v1_interface,
                    version));
    }

    void initSeat(struct ::wl_registry *registry, int id, int version) {
        seat_ = static_cast<wl_seat *>(
            wl_registry_bind(registry, id, &wl_seat_interface, version));
    }

    struct wl_display *display_;
    struct wl_registry *registry_;
    struct wl_seat *seat_;
    struct zwp_primary_selection_device_manager_v1 *device_manager_;
    struct zwp_primary_selection_device_v1 *device_;
    struct wl_registry_listener registry_listener_;
    struct zwp_primary_selection_device_v1_listener device_listener_;
    struct zwp_primary_selection_offer_v1_listener offer_listener_;
    QString selection_;
    // QMimeData mime_date_;
};

#endif  // WAYLAND_PRIMARY_SELECTION_H