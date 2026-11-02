# FerruxeBass

**FerruxeBass** é un plugin directo para darlle peso, corpo e ese punto xusto de *ferruxe* e agarre ao baixo eléctrico. Inspirado en pedáis tipo **SansAmp**, mantén uns graves firmes que asentan a mestura mentres engade carácter.

## Controis

`INPUT` ➔ `THRESH` ➔ `SUB-LOWS` + `DRIVE` ➔ `OUTPUT` ➔ `CEILING`

* **Input (`-24` a `+24 dB`):** Ganancia de entrada para axustar con canta forza o baixo empuxa o circuíto.
* **Thresh (`-40` a `0 dB`):** Compresor previo á distorsión que iguala o toque de púa ou dedos para que todas as notas soen firmes.
* **Sub-Lows (`0` a `100 %`):** Engade corpo e harmónicos graves estilo *RBass* para que o baixo se sinta fondo mesmo en altofalantes pequenos.
* **Drive (`0` a `100 %`):** Saturación con carácter estilo *SansAmp* que achega ruxido, ataque e ese agarre metálico sen apagar o subgrave limpo.
* **Output (`-24` a `+24 dB`):** Volume xeral de saída.
* **Ceiling (`-24` a `0 dB`):** Recortador (*hard clipper*) final para domar os picos rebeldes e cravar o teito de saída.

## Formatos e Compilación (Linux)

Desenvolvido en C++20 con **JUCE**. Xera formatos nativos **LV2**, **VST3** e **Standalone** (JACK / PipeWire / ALSA).

```bash
cmake -B build -G Ninja -DJUCE_DIR="$HOME/opt/JUCE"
cmake --build build --parallel
```

Pido perdón aos developers de c++ por esta aberración