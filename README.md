# SplashScreenNG
Customizable Splash Screen for Skyrim SE/AE/GOG/VR

## Requirements
- [Visual Studio 2022](https://visualstudio.microsoft.com/)
- [`vcpkg`](https://github.com/microsoft/vcpkg)

## Configuration
Place `config.json` in `Data/SKSE/Plugins/SplashScreenNG/`. All fields are optional and will fall back to defaults if omitted.

### Window
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `width` | int | `835` | Splash window width in pixels |
| `height` | int | `400` | Splash window height in pixels |
| `opacity` | float | `1` | Overall window opacity (`0.0`–`1.0`) |
| `windowStyle` | string | `"forced"` | Window style preset — see [Window Styles](#window-styles) |

### Splash Image
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `splash` | string | `"splash.png"` | Filename of the background image (relative to the plugin folder) |
| `fadeIn` | bool | `true` | Fade in on open |
| `fadeOut` | bool | `true` | Fade out on close |
| `fadeStep` | float | `0.02` | Opacity increment — lower is slower |

### Spinner
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `useSpinner` | bool | `true` | Show an animated spinner |
| `spinner` | string | `"loading.gif"` | Filename of the spinner image |
| `spinnerX` | int | `780` | Spinner X position in pixels |
| `spinnerY` | int | `350` | Spinner Y position in pixels |
| `spinnerWidth` | int | `32` | Spinner display width in pixels |
| `spinnerHeight` | int | `32` | Spinner display height in pixels |

### Text
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `useText` | bool | `true` | Show loading log text at the bottom of the splash |
| `textFont` | string | `"Segoe UI Semibold"` | Font family name |
| `textFontSize` | float | `12` | Font size in points |
| `textPadding` | int | `80` | Bottom padding for the text in pixels |
| `textColorR` | int | `255` | Text color red channel (`0`–`255`) |
| `textColorG` | int | `255` | Text color green channel (`0`–`255`) |
| `textColorB` | int | `255` | Text color blue channel (`0`–`255`) |

### Behaviour
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `forceFocus` | bool | `true` | Attempt to force Skyrim to the front |
| `closeOn` | int | `8` | SKSE load stage at which the splash closes. 6 = kInputLoaded, 8 = kDataLoaded |

### Window Styles
The `windowStyle` key accepts a preset name. You can also override the raw Win32 style values directly under `windowStyles`.

| Preset | Value | Description | Win32 Style
|--------|-------|-------------|--------------|
| `forced` | `134217736` | Topmost, no taskbar icon _(default)_ | `WS_EX_TOPMOST \| WS_EX_NOACTIVATE` | 
| `forcedicon` | `524296` | Topmost with taskbar icon | `WS_EX_TOPMOST` |
| `normal` | `0` | Standard window | None |