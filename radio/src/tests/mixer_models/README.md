# Mixer test models

Templates by Mike Shellim (https://rc-soar.com/edgetx/), used with his
permission. They chain many virtual channels, which makes them good test
cases for the mixer's channel dependency handling.

| File | Template |
|---|---|
| `esoarplus.yml` | E-Soar Plus 4.0.0, X-tail variant |
| `f3f.yml` | F3F 6.2.0, V-tail variant |
| `dlg6s.yml` | DLG 6S 2.2.0 |
| `esoarplus310.yml` | E-Soar Plus 3.1.0, V-tail variant: 58 mixes; the model behind the startup stutter of #5976 |

The files are the original models from the templates' `.etx` packages
(E-Soar Plus 3.1.0: as sent by Mike Shellim, saved by EdgeTX 2.12.5), with
a blanked `modelRegistrationID`.
