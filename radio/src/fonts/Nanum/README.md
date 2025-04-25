# NanumBarunpen Korean Font Integration for EdgeTX

This folder provides Korean font support for EdgeTX using the [NanumBarunpen](https://hangeul.naver.com/fonts/search?f=nanum) family,
licensed under the [SIL Open Font License 1.1](https://scripts.sil.org/OFL).

## Fonts Used
- `NanumBarunpenR.ttf` – Regular style for standard text
- `NanumBarunpenB.ttf` – Bold style (optional use)

## Font Sizes Included
- `lv_font_ko_XS.c`: Small text (e.g., channel names, telemetry values)
- `lv_font_ko_STD.c`: Default UI text (e.g., menus, labels)
- `lv_font_ko_L.c`: Large text (e.g., timers, status bars)

## License
See [LICENSE.txt](./LICENSE.txt) for full license text.
This font is licensed under the **SIL Open Font License 1.1**.

## Notes
- Only `NanumBarunpen` fonts are used, even though the license lists other Nanum font families.
- Fonts are embedded as `.c` source files using `lv_font_conv` and do not require runtime rendering or `.ttf` dependencies.
