# MSPM0-Embedded-Cooking-Game

Collaboratively developed by **Qasim Bawa** and **Zuhayr Khondker** on the MSPM0G3507 microcontroller as a real-time interactive embedded systems project combining hardware interfacing, game logic, graphics, and audio.

## Demo Video

https://www.youtube.com/watch?v=sAzIugYzNG0

## Overview

Our version of a cooking game challenges the player to control a chef using a joystick and buttons to collect, prepare, and combine ingredients into soups before time runs out. Players must move efficiently, complete the correct preparation steps, and maximize score through fast decision-making and route planning.

The project integrates multiple embedded systems concepts, including ADC-based joystick input, GPIO buttons, SPI LCD graphics, DAC sound playback, interrupts, finite state machine control, and bilingual menu support.

## Features

- Joystick-controlled chef movement
- Tile-based collision and object interaction
- Ingredient pickup/washing/chopping mechanics
- Recipe-based scoring system
- Sound effects using DAC + SysTick interrupt
- Two-language support (English / Spanish)
- Pause menu and restart flow
- Real-time countdown gameplay timer

## Gameplay Objects

| Object | Function |
|-------|----------|
| Chef | Player-controlled character |
| Ingredients | Collect and prepare for recipes |
| Pot | Submit ingredients for soup |
| Prep Station | Wash / chop ingredients |
| Recipe Tile | Displays valid recipes |

## Recipes

| Soup | Requirements | Points |
|------|-------------|--------|
| Yellow Soup | Chopped tomato + chopped mushroom + chopped carrot | 20 |
| Purple Soup | Chopped tomato + raw mushroom | 10 |

## Architecture

- Controller FSM (Title / Language / Instructions / Playing / Pause / Recipe / Game Over)
- ADC joystick input system
- GPIO button input with debounce logic
- SysTick interrupt for sampled audio playback
- Timer interrupt for button timing / debounce
- Tile-based rendering engine
- Sprite collision detection
- Score + timer HUD

## Team Contributions

- **Qasim Bawa**: Embedded software development, gameplay systems, hardware integration, debugging, testing
- **Zuhayr Khondker**: Game design, embedded implementation, graphics/assets integration, testing, system refinement

## Files

```
Overcooked-ECE319K/
├── src/                 Game code
│   ├── main.c           Program entry point
│   ├── game.c / game.h  Game logic, rendering, controller FSM
│   ├── Sound.c / .h     DAC sound playback (SysTick)
│   ├── Switch.c / .h    GPIO button input
│   ├── images/images.h  Sprite and tile bitmaps
│   └── sounds/sounds.h  Sampled audio data
├── inc/                 Hardware drivers and libraries
│   ├── ADC1.c / .h      Joystick ADC input
│   ├── DAC.c / .h       DAC output
│   ├── Clock, LaunchPad, SPI, Timer (.c / .h)
│   └── ST7735, SmallFont (.c / .h)   LCD driver and font
├── config/              Linker script and startup file (MSPM0G3507)
├── LICENSE
└── README.md
```

## Tools

- Embedded C
- TI Code Composer Studio
- MSPM0G3507 LaunchPad
- ST7735 LCD
- ADC
- DAC
- GPIO
- Timer Interrupts
- SysTick
