[简体中文](./README.zh-CN.md)

# ![](./data/io.github.glaumar.PopTranslate.png) PopTranslate
PopTranslate is a translator running under wayland, which can translate the text selected by the mouse and display the result in a pop-up window (only Plasma Wayland is currently supported)

# Why write yet another translator?
GoldenDict, Crow Translate and other translation software are very useful, but due to the wayland protocol restrictions, they cannot register global shortcut keys, and cannot read the clipboard and mouse position in the background (resulting in the inability to pop up a window near the mouse and translate)
# Screenshots

![](./screeshots/Screenshot1.png)

![](./screeshots/Screenshot2.png)

![](./screeshots/Screenshot3.png)

# Global Shortcuts
`Meta + G` or `Meta + Ctrl + G` to the text selected by the mouse and display a popup


# INSTALL
## Arch Linux (AUR)
```bash
paru -S poptranslate
```

# Dependencies
- [CMake](https://cmake.org/) >= 3.5
- [Extra CMake Modules](https://github.com/KDE/extra-cmake-modules)
- [Qt](https://www.qt.io/) >= 5.15
- [KDE Frameworks](https://api.kde.org/frameworks/index.html) >= 5.108 with at least the following modules:
    - KGlobalAccel
    - KGuiAddons
    - KWayland
    - KWindowSystem
    - KWidgetsAddons
    - KXmlGui
- [QOnlineTranslator](https://github.com/crow-translate/QOnlineTranslator) - provides free usage of Google, Yandex and Bing translate API

## Flatpak Runtime and SDK
- org.kde.Platform = "5.15-22.08"
- org.kde.Sdk = "5.15-22.08"

# Build and install locally using flatpak

## Install runtime and sdk
```bash
flatpak install org.kde.Platform/x86_64/5.15-22.08
flatpak install org.kde.Sdk/x86_64/5.15-22.08
```

## Build and install
```bash
flatpak-builder build-dir io.github.glaumar.PopTranslate.yml --force-clean --user --install
```
