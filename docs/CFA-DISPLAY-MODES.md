# Kaleido 3 Hardware CFA Display Modes (`4.44.23552`)

Kobo Libra Colour (`mt8113t-ntx`) exposes hardware Color Filter Array (CFA) LUT modes through `DevSettings::cfaMode()` (`0x00a18560`) and `DevSettings::setCfaMode(int)` (`0x00a18600`), backed by the `[DeveloperSettings]` key `ForceCfaMode` in `/mnt/onboard/.kobo/Kobo/Kobo eReader.conf`.

Disassembling `N3SettingsDeveloperOptionsView::N3SettingsDeveloperOptionsView` (`0x01043eec`–`0x0104417a` in `libnickel.so.1.0.0`, build `4.44.23552`) yields the exact integer constants written to `ForceCfaMode`:

| UI Label (`CFA Mode:`) | Decimal | Hex    | Notes |
|------------------------|--------:|--------|-------|
| `Default`              | `0`     | `0x00` | Stock factory profile |
| `Skip`                 | `155`   | `0x9b` | Bypass CFA post-processing |
| `Normal (G1)`          | `156`   | `0x9c` | Standard G1 curve |
| `S4`                   | `157`   | `0x9d` | Saturation curve S4 |
| `S7`                   | `158`   | `0x9e` | High saturation curve S7 |
| `S9`                   | `160`   | `0xa0` | Maximum saturation curve S9 |
| `G0`                   | `161`   | `0xa1` | Gamma G0 |
| `G1`                   | `162`   | `0xa2` | Explicit Gamma G1 |
| **`G2`**               | **`163`** | **`0xa3`** | **Deeper gamma & richer color contrast on book covers and highlights** |

## Persistent Configuration

While Nickel is stopped (or via the Developer Options UI unlocked by `EnableDebugServices=true` / typing `devmodeon` in Search):

```ini
[DeveloperSettings]
EnableDebugServices=true
ForceCfaMode=163
```
