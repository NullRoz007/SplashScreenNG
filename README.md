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
| `fullscreen` | bool | `false` | Stretch the splash window to fill the screen |
| `forceFocus` | bool | `false` | Force the splash window to take focus |
| `draggable` | bool | `false` | Allow the splash window to be dragged |

### Splash Image
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `splash` | string | `"splash.png"` | Filename of the background image (relative to the plugin folder) |
| `randomSplash` | bool | `false` | Pick a random `.png` from `randomSplashFolder` instead of using `splash` |
| `randomSplashFolder` | string | `"splashes"` | Folder to pick a random splash from (relative to the plugin folder) |
| `fadeIn` | bool | `true` | Fade in on open |
| `fadeOut` | bool | `true` | Fade out on close |
| `fadeStep` | float | `0.02` | Opacity increment per tick — lower is slower |

### Spinner
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `useSpinner` | bool | `true` | Show an animated spinner |
| `spinner` | string | `"loading.gif"` | Filename of the spinner image (relative to the plugin folder) |
| `spinnerX` | int | `780` | Spinner X position in pixels |
| `spinnerY` | int | `350` | Spinner Y position in pixels |
| `spinnerWidth` | int | `32` | Spinner display width in pixels |
| `spinnerHeight` | int | `32` | Spinner display height in pixels |

### Text
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `useText` | bool | `true` | Show loading log text on the splash |
| `textFont` | string | `"Segoe UI Semibold"` | Font family name |
| `textFontSize` | float | `12` | Font size in points |
| `textPadding` | int | `80` | Padding around the text in pixels |
| `textColorR` | int | `255` | Text color red channel (`0`–`255`) |
| `textColorG` | int | `255` | Text color green channel (`0`–`255`) |
| `textColorB` | int | `255` | Text color blue channel (`0`–`255`) |
| `textX` | int | `0` | Text X position in pixels |
| `textY` | int | `0` | Text Y position in pixels |
| `textWeight` | string | `"normal"` | Font weight preset — see [Text Weights](#text-weights) |
| `textStyle` | string | `"normal"` | Font style preset — see [Text Styles](#text-styles) |
| `textAlignment` | string | `"center"` | Text alignment preset — see [Text Alignment](#text-alignment) |

### Cursor
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `useCursor` | bool | `false` | Display the game's cursor loaded from `interface/menucursor.swf` |
| `cursorScale` | float | `1.0` | Scale factor for the game cursor size |

### Behaviour
| Key | Type | Default | Description |
|-----|------|---------|-------------|
| `closeOn` | int | `8` | SKSE load stage at which the splash closes. `6` = kInputLoaded, `8` = kDataLoaded |

---

### Window Styles
The `windowStyle` key accepts a preset name. You can override the raw Win32 style values directly under `windowStyles`.

| Preset | Value | Description | Win32 Style |
|--------|-------|-------------|-------------|
| `forced` | `134217736` | Topmost, no taskbar icon _(default)_ | `WS_EX_TOPMOST \| WS_EX_NOACTIVATE` |
| `forcedicon` | `8` | Topmost with taskbar icon | `WS_EX_TOPMOST` |
| `normal` | `0` | Standard window | None |

### Text Weights
The `textWeight` key accepts a preset name. You can override the raw DirectWrite weight values directly under `textWeights`.

| Preset | Value | Description |
|--------|-------|-------------|
| `normal` | `400` | Regular weight _(default)_ |
| `bold` | `700` | Bold weight |

### Text Styles
The `textStyle` key accepts a preset name. You can override the raw DirectWrite style values directly under `textStyles`.

| Preset | Value | Description |
|--------|-------|-------------|
| `normal` | `0` | Upright text _(default)_ |
| `italic` | `2` | Italic text |

### Text Alignment
The `textAlignment` key accepts a preset name. You can override the raw DWRITE text alignment values directly under `textAlignments`. Paragraph alignment is selected automatically. Invalid values fall back to centered alignment.

| Preset | Value | Paragraph Alignment | DWRITE Text Align |
|--------|-------|---------------------|-------------------|
| `left` | `0` | Near | `LEADING` |
| `center` | `2` | Center | `CENTER` |
| `right` | `1` | Far | `TRAILING` |