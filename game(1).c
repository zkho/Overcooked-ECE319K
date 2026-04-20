#include <stdint.h>
#include <stdbool.h>
#include <ti/devices/msp/msp.h>
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/ST7735.h"
#include "../inc/ADC1.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
#include "../inc/Timer.h"

// ---------------- constants ----------------
#define TILE        16
#define GRID_W      8
#define GRID_H      10

#define BLACK_BTN   0x02  
#define WHITE_BTN   0x01  

#define TITLE_TICKS         50      
#define HOME_TICK_MS        100
#define GAME_SECONDS        120
#define GAME_TICKS          (GAME_SECONDS*10)   
#define MOVE_COOLDOWN_TICKS 2
#define FLASH_TICKS         8      


#define COLOR_FLOOR_A   0xFFFF
#define COLOR_FLOOR_B   0xC618
#define COLOR_TABLE     0xA145
#define COLOR_RECIPE    0x7DFF
#define COLOR_TEXT_BG   0x0000
#define COLOR_TEXT_FG   0xFFFF
#define COLOR_HILITE    0xFFE0
#define COLOR_BOX       0x0000

// ---------------- enums ----------------
typedef enum{
  LANG_EN = 0,
  LANG_ES = 1
} Language_t;

typedef enum{
  STATE_TITLE = 0,
  STATE_LANGUAGE,
  STATE_HOME,
  STATE_INSTRUCTIONS,
  STATE_PLAYING,
  STATE_PAUSED,
  STATE_GAMEOVER,
  STATE_RECIPE
} GameState_t;

typedef enum{
  FACE_LEFT = 0,
  FACE_RIGHT
} Facing_t;

typedef enum{
  ITEM_NONE = 0,
  ITEM_TOMATO,
  ITEM_TOMATO_CHOP,
  ITEM_MUSHROOM,
  ITEM_MUSHROOM_CHOP,
  ITEM_CARROT,
  ITEM_CARROT_CHOP
} Item_t;


typedef enum{
  OBJ_NONE = 0,
  OBJ_TOMATO,
  OBJ_MUSHROOM,
  OBJ_CARROT,
  OBJ_POT,
  OBJ_RECIPE,
  OBJ_PREP0,   
  OBJ_PREP1,
  OBJ_PREP2,
  OBJ_PREP3
} Object_t;

// ---------------- globals ----------------
static Language_t Language = LANG_EN;
static GameState_t State = STATE_TITLE;
static Facing_t ChefFacing = FACE_RIGHT;

volatile uint8_t ButtonLock = 0;
volatile uint8_t ButtonTicks = 0;

static int ChefCol = 3;
static int ChefRow = 5;

static uint32_t TitleCounter = TITLE_TICKS;
static uint32_t GameCounter = GAME_TICKS;
static uint32_t MoveCooldown = 0;
static uint32_t Score = 0;

static uint32_t PrevButtons = 0;
static uint8_t PotReady = 0; 
static uint32_t PotReadyPoints = 0;

static Item_t HeldItem = ITEM_NONE;

static uint8_t HeldWashed = 0;

static Item_t StoredItem1 = ITEM_NONE;
static Item_t StoredItem2 = ITEM_NONE;
static uint8_t StoredWashed1 = 0;
static uint8_t StoredWashed2 = 0;

static uint8_t PotTomato = 0;
static uint8_t PotTomatoChop = 0;
static uint8_t PotMushroom = 0;
static uint8_t PotMushroomChop = 0;
static uint8_t PotCarrot = 0;
static uint8_t PotCarrotChop = 0;

// map of blocked interactable tiles
static Object_t Map[GRID_H][GRID_W] = {
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE},
  {OBJ_TOMATO,  OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_POT},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE},
  {OBJ_MUSHROOM,OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_PREP0},
  {OBJ_CARROT,  OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_PREP1},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_PREP2},
  {OBJ_NONE,    OBJ_NONE, OBJ_NONE, OBJ_RECIPE, OBJ_NONE, OBJ_NONE, OBJ_NONE, OBJ_PREP3}
};

// ---------------- prototypes ----------------
static void DrawSpriteTransparent(int x, int y, const uint16_t *img, int w, int h);
static void DrawTileBg(int col, int row);
static void DrawStoredItems(void);
static void DrawObjectAt(int col, int row);
static void DrawBoard(void);
static void RedrawCell(int col, int row);
static void DrawChef(void);
static void DrawHeldItem(void);
static void DrawHUD(void);
static void DrawHomeOverlay(void);
static void DrawLanguageScreen(int sel);
static void DrawGameOverScreen(void);
static void ResetGame(void);
static int  IsWalkable(int col, int row);
static void MoveChef(int dcol, int drow);
static int  FindAdjacentObject(int *oc, int *orow, Object_t *obj);
static void HandleInteract(void);
static void ShowRecipeOverlay(void);
static bool ChopSequence(void);
static void AddHeldToPot(void);
static uint8_t CheckCompletedSoup(void);
static void ClearPot(void);
static uint32_t ReadBlackPressEdge(uint32_t buttons);
static uint32_t ReadWhitePressEdge(uint32_t buttons);
static bool MovementRequested(uint32_t x, uint32_t y);


static uint32_t ReadBlackPressEdge(uint32_t buttons){
  return ((buttons & BLACK_BTN) && !(PrevButtons & BLACK_BTN));
}
static uint32_t ReadWhitePressEdge(uint32_t buttons){
  return ((buttons & WHITE_BTN) && !(PrevButtons & WHITE_BTN));
}

static bool MovementRequested(uint32_t x, uint32_t y){
  return (x < 1000 || x > 3000 || y < 1000 || y > 3000);
}


static void DrawSpriteTransparent(int x, int y, const uint16_t *img, int w, int h){
  int i, j;
  for(i = 0; i < h; i++){
    for(j = 0; j < w; j++){
      uint16_t pixel = img[(h - 1 - i)*w + j];
      if(pixel != 0xFFFF){  
        ST7735_DrawPixel(x + j, y + i, pixel);
      }
    }
  }
}

static void DrawTileBg(int col, int row){
  uint16_t color;


  if(Map[row][col] == OBJ_TOMATO || Map[row][col] == OBJ_MUSHROOM || Map[row][col] == OBJ_CARROT){
    color = COLOR_TABLE;
  }else if(Map[row][col] == OBJ_RECIPE){
    color = COLOR_RECIPE;
  }else if(Map[row][col] == OBJ_PREP0 || Map[row][col] == OBJ_PREP1 || Map[row][col] == OBJ_PREP2 || Map[row][col] == OBJ_PREP3){
    color = 0x0000;
  }else{
    color = ((row + col) & 1) ? COLOR_FLOOR_A : COLOR_FLOOR_B;
  }

  ST7735_FillRect(col*TILE, row*TILE, TILE, TILE, color);
}

static void DrawInstructionsScreen(void){
  ST7735_FillScreen(0x0000);

  ST7735_SetCursor(0,1);
  ST7735_OutString("INSTR.");

  // yellow pot = 20
  DrawSpriteTransparent(0, 24, aaa_yellowpot_16x16, 16, 16);
  ST7735_SetCursor(3,3);
  ST7735_OutString("=20");

  // purple pot = 10
  DrawSpriteTransparent(0, 54, aaa_purplepot_16x16, 16, 16);
  ST7735_SetCursor(3,6);
  ST7735_OutString("=10");

  // washing rule
  ST7735_SetCursor(0,9);
  ST7735_OutString("no wash/sin lavado");
  ST7735_SetCursor(0,11);
  ST7735_OutString("->pot reset/reinicia");


  // start
  ST7735_SetCursor(0,15);
  ST7735_OutString("B:start");
}

static void DrawRecipeTile(int col, int row){
  int x = col*TILE;
  int y = row*TILE;
  uint16_t c = 0x0000; 

  DrawTileBg(col,row);

  for(int i=2; i<=13; i++){
    ST7735_DrawPixel(x+4, y+i, c);
    ST7735_DrawPixel(x+5, y+i, c);
  }

  for(int i=4; i<=10; i++){
    ST7735_DrawPixel(x+i, y+2, c);
    ST7735_DrawPixel(x+i, y+3, c);
  }

  for(int i=4; i<=10; i++){
    ST7735_DrawPixel(x+i, y+7, c);
    ST7735_DrawPixel(x+i, y+8, c);
  }

  for(int i=3; i<=7; i++){
    ST7735_DrawPixel(x+10, y+i, c);
    ST7735_DrawPixel(x+11, y+i, c);
  }

  ST7735_DrawPixel(x+7, y+9, c);
  ST7735_DrawPixel(x+8, y+10, c);
  ST7735_DrawPixel(x+9, y+11, c);
  ST7735_DrawPixel(x+10, y+12, c);
  ST7735_DrawPixel(x+11, y+13, c);
}

static void DrawObjectAt(int col, int row){
  int x = col*TILE;
  int y = row*TILE;

  switch(Map[row][col]){
    case OBJ_TOMATO:
      DrawSpriteTransparent(x+3, y+3, aaa_tomato_10x10, 10, 10);
      break;
    case OBJ_MUSHROOM:
      DrawSpriteTransparent(x+3, y+3, aaa_mushroom_10x10, 10, 10);
      break;
    case OBJ_CARROT:
      DrawSpriteTransparent(x+3, y+3, aaa_carrot_10x10, 10, 10);
      break;
    case OBJ_POT:
        if(PotReady == 1){
            DrawSpriteTransparent(x, y, aaa_purplepot_16x16, 16, 16);
        }else if(PotReady == 2){
            DrawSpriteTransparent(x, y, aaa_yellowpot_16x16, 16, 16);
        }else{
            DrawSpriteTransparent(x, y, aaa_emptypot_16x16, 16, 16);
        }
    break;
    case OBJ_PREP0:
      DrawSpriteTransparent(x, y, aaa_prepstation_16x64, 16, 64);
      break;
    case OBJ_RECIPE:
      DrawRecipeTile(col,row);
      break;
    default:
      break;
  }
}

static void DrawBoard(void){
  int r,c;
  ST7735_FillScreen(0x0000);
  for(r=0;r<GRID_H;r++){
    for(c=0;c<GRID_W;c++){
      DrawTileBg(c,r);
      if(Map[r][c] != OBJ_PREP0 && Map[r][c] != OBJ_PREP1 && Map[r][c] != OBJ_PREP2 && Map[r][c] != OBJ_PREP3){
        DrawObjectAt(c,r);
      }
    }
  }
  DrawSpriteTransparent(7*TILE, 6*TILE, aaa_prepstation_16x64, 16, 64);
  DrawStoredItems();
  DrawChef();
  DrawHeldItem();
  DrawHUD();
}

static void RedrawCell(int col, int row){
  DrawTileBg(col,row);
  if(Map[row][col] == OBJ_PREP0 || Map[row][col] == OBJ_PREP1 || Map[row][col] == OBJ_PREP2 || Map[row][col] == OBJ_PREP3){
    DrawSpriteTransparent(7*TILE, 6*TILE, aaa_prepstation_16x64, 16, 64);
    DrawStoredItems();
  }else{
    DrawObjectAt(col,row);
  }
}

static void DrawChef(void){
  int x = ChefCol*TILE;
  int y = ChefRow*TILE;

  if(ChefFacing == FACE_LEFT){
    DrawSpriteTransparent(x, y, aaa_chef_left_16x16, 16, 16);
  }else{
    DrawSpriteTransparent(x, y, aaa_chef_right_16x16, 16, 16);
  }
}

static void DrawHeldItem(void){
  const uint16_t *img = 0;
  int w = 10, h = 10;
  int x, y;

  switch(HeldItem){
    case ITEM_TOMATO:       img = aaa_tomato_10x10; break;
    case ITEM_TOMATO_CHOP:  img = aaa_tomato_chopped_10x10; break;
    case ITEM_MUSHROOM:     img = aaa_mushroom_10x10; break;
    case ITEM_MUSHROOM_CHOP:img = aaa_mushroom_chopped_10x10; break;
    case ITEM_CARROT:       img = aaa_carrot_10x10; break;
    case ITEM_CARROT_CHOP:  img = aaa_carrot_chopped_10x10; break;
    default: return;
  }

  if(ChefFacing == FACE_LEFT){
    x = ChefCol*TILE - 2;
  }else{
    x = ChefCol*TILE + 8;
  }
  y = ChefRow*TILE + 3;

  DrawSpriteTransparent(x, y, img, w, h);

  if(HeldWashed){
    ST7735_DrawPixel(x+9, y, 0x001F);
  }
}

static void DrawHUD(void){
  char *pts = (Language == LANG_EN) ? "Pts:" : "Pts:";
  char *tmr = (Language == LANG_EN) ? "Time:" : "Tiempo:";
  ST7735_FillRect(0, 0, 128, 12, 0x0000);
  ST7735_SetCursor(0,0);
  ST7735_OutString(pts);
  ST7735_OutUDec(Score);

  ST7735_SetCursor(10,0);
  ST7735_OutString(tmr);
  ST7735_OutUDec(GameCounter/10);
}

static void DrawLanguageScreen(int sel){
  ST7735_FillScreen(0x0000);
  ST7735_SetCursor(1,2);
  ST7735_OutString("Choose language");

  ST7735_SetCursor(3,6);
  if(sel == 0) ST7735_OutString("> English <");
  else         ST7735_OutString("  English  ");

  ST7735_SetCursor(3,9);
  if(sel == 1) ST7735_OutString("> Spanish <");
  else         ST7735_OutString("  Spanish  ");

  ST7735_SetCursor(0,13);
  ST7735_OutString("Black: confirm");
  ST7735_SetCursor(0,14);
  ST7735_OutString("White: back");
}

static void DrawHomeOverlay(void){
  ST7735_FillRect(0, 112, 128, 48, 0x0000);
  ST7735_SetCursor(0,14);

  if(Language == LANG_EN){
    ST7735_OutString("Black: begin game");
    ST7735_SetCursor(0,15);
    ST7735_OutString("White: change lang");
  }else{
    ST7735_OutString("Negro: comenzar");
    ST7735_SetCursor(0,15);
    ST7735_OutString("Blanco: idioma");
  }
}

static void DrawPauseOverlay(void){
  ST7735_FillRect(8, 48, 112, 48, 0x0000);
  ST7735_SetCursor(3,7);
  if(Language == LANG_EN){
    ST7735_OutString("PAUSED");
    ST7735_SetCursor(0,9);
    ST7735_OutString("White: resume");
    ST7735_SetCursor(0,10);
    ST7735_OutString("Black: end game");
  }else{
    ST7735_OutString("PAUSA");
    ST7735_SetCursor(0,9);
    ST7735_OutString("Blanco: seguir");
    ST7735_SetCursor(0,10);
    ST7735_OutString("Negro: terminar");
  }
}
static void DrawGameOverScreen(void){
  ST7735_FillScreen(0x0000);

  for(int r=0;r<GRID_H;r++){
    for(int c=0;c<GRID_W;c++){
      Object_t tmp = Map[r][c];
      Map[r][c] = OBJ_NONE;
      DrawTileBg(c,r);
      Map[r][c] = tmp;
    }
  }

  ST7735_SetCursor(0,3);
  if(Language == LANG_EN){
    ST7735_OutString("Points scored: ");
  }else{
    ST7735_OutString("Puntos: ");
  }
  ST7735_OutUDec(Score);

  ST7735_SetCursor(0,6);
  if(Language == LANG_EN){
    ST7735_OutString("Wow, that's a lot");
    ST7735_SetCursor(0,7);
    ST7735_OutString("of soup. Good job.");
    ST7735_SetCursor(0,10);
    ST7735_OutString("White: play again");
    ST7735_SetCursor(0,11);
    ST7735_OutString("Black: home screen");
  }else{
    ST7735_OutString("Mucha sopa.");
    ST7735_SetCursor(0,7);
    ST7735_OutString("Buen trabajo.");
    ST7735_SetCursor(0,10);
    ST7735_OutString("Blanco: jugar otra");
    ST7735_SetCursor(0,11);
    ST7735_OutString("Negro: inicio");
  }
}

void TIMG0_IRQHandler(void){
  if((TIMG0->CPU_INT.IIDX) == 1){ // acknowledge interrupt
    if(ButtonLock){
      ButtonTicks++;
      if(ButtonTicks >= 10){   // 10 * 50ms = 500ms
        ButtonLock = 0;
        ButtonTicks = 0;
      }
    }
  }
}

// ---------------- gameplay helpers ----------------
static void ClearPot(void){
  PotTomato = 0;
  PotTomatoChop = 0;
  PotMushroom = 0;
  PotMushroomChop = 0;
  PotCarrot = 0;
  PotCarrotChop = 0;
}

static uint8_t CheckCompletedSoup(void){
  if(PotTomatoChop && PotMushroomChop && PotCarrotChop){
    return 2;
  }
  if(PotTomatoChop && PotMushroom){
    return 1;
  }
  return 0;
}

static void AddHeldToPot(void){
  uint8_t soup = 0;

  if(PotReady != 0) return; 

  if(HeldWashed == 0){
    ClearPot();
    HeldItem = ITEM_NONE;
    HeldWashed = 0;
    RedrawCell(7,1);
    DrawChef();
    DrawHeldItem();
    DrawHUD();
    return;
  }


  switch(HeldItem){
    case ITEM_TOMATO:         PotTomato = 1; break;
    case ITEM_TOMATO_CHOP:    PotTomatoChop = 1; break;
    case ITEM_MUSHROOM:       PotMushroom = 1; break;
    case ITEM_MUSHROOM_CHOP:  PotMushroomChop = 1; break;
    case ITEM_CARROT:         PotCarrot = 1; break;
    case ITEM_CARROT_CHOP:    PotCarrotChop = 1; break;
    default: return;
  }

  HeldItem = ITEM_NONE;
  HeldWashed = 0;
  Sound_Splash();

  soup = CheckCompletedSoup();
  if(soup == 1){
    PotReady = 1;          
    PotReadyPoints = 10;
    Sound_Complete();
  }else if(soup == 2){
    PotReady = 2;        
    PotReadyPoints = 20;
    Sound_Complete();
  }

  RedrawCell(7,1);
  DrawChef();
  DrawHeldItem();
  DrawHUD();
}

static void CollectPotSoup(void){
  if(PotReady == 0) return;

  Score += PotReadyPoints;
  PotReady = 0;
  PotReadyPoints = 0;
  ClearPot();

  RedrawCell(7,1);
  DrawChef();
  DrawHeldItem();
  DrawHUD();
}

static int IsWalkable(int col, int row){
  if(col < 0 || col >= GRID_W || row < 0 || row >= GRID_H) return 0;
  if(row == 0) return 0;
  return (Map[row][col] == OBJ_NONE);
}

static void MoveChef(int dcol, int drow){
  int newCol = ChefCol + dcol;
  int newRow = ChefRow + drow;

  if(dcol < 0) ChefFacing = FACE_LEFT;
  if(dcol > 0) ChefFacing = FACE_RIGHT;

  if(!IsWalkable(newCol, newRow)) return;

  RedrawCell(ChefCol, ChefRow);
  ChefCol = newCol;
  ChefRow = newRow;
  DrawChef();
  DrawHeldItem();
}

static int FindAdjacentObject(int *oc, int *orow, Object_t *obj){
  int c = ChefCol, r = ChefRow;

  if(c-1 >= 0 && Map[r][c-1] != OBJ_NONE){ *oc = c-1; *orow = r; *obj = Map[r][c-1]; return 1; }
  if(c+1 < GRID_W && Map[r][c+1] != OBJ_NONE){ *oc = c+1; *orow = r; *obj = Map[r][c+1]; return 1; }
  if(r-1 >= 0 && Map[r-1][c] != OBJ_NONE){ *oc = c; *orow = r-1; *obj = Map[r-1][c]; return 1; }
  if(r+1 < GRID_H && Map[r+1][c] != OBJ_NONE){ *oc = c; *orow = r+1; *obj = Map[r+1][c]; return 1; }

  return 0;
}

static bool ChopSequence(void){
  int i;
  uint32_t x, y;
  const uint16_t *knifeUp;
  const uint16_t *knifeDown;

  if(HeldItem == ITEM_NONE) return false;
  if(HeldItem == ITEM_TOMATO_CHOP || HeldItem == ITEM_MUSHROOM_CHOP || HeldItem == ITEM_CARROT_CHOP) return false;
  
  knifeUp = (ChefFacing == FACE_LEFT) ? aaa_chef_left_knifeup_16x16 : aaa_chef_right_knifeup_16x16;
  knifeDown = (ChefFacing == FACE_LEFT) ? aaa_chef_left_knifedown_16x16 : aaa_chef_right_knifedown_16x16;
  

  for(i=0;i<3;i++){
    ADCin2(&x,&y);
    if(MovementRequested(x,y)) return false;
    RedrawCell(ChefCol, ChefRow);
    DrawSpriteTransparent(ChefCol*TILE, ChefRow*TILE, knifeUp, 16, 16);
    DrawHeldItem();
    Clock_Delay1ms(200);

    ADCin2(&x,&y);
    if(MovementRequested(x,y)) return false;
    RedrawCell(ChefCol, ChefRow);
    DrawSpriteTransparent(ChefCol*TILE, ChefRow*TILE, knifeDown, 16, 16);
    Sound_Chop();
    DrawHeldItem();
    Clock_Delay1ms(200);
  }

  if(HeldItem == ITEM_TOMATO) HeldItem = ITEM_TOMATO_CHOP;
  else if(HeldItem == ITEM_MUSHROOM) HeldItem = ITEM_MUSHROOM_CHOP;
  else if(HeldItem == ITEM_CARROT) HeldItem = ITEM_CARROT_CHOP;
  RedrawCell(ChefCol, ChefRow);
  DrawChef();
  DrawHeldItem();
  return true;
}

static void DrawStoredItems(void){
  const uint16_t *img = 0;

  if(StoredItem1 != ITEM_NONE){
    switch(StoredItem1){
      case ITEM_TOMATO: img = aaa_tomato_10x10; break;
      case ITEM_TOMATO_CHOP: img = aaa_tomato_chopped_10x10; break;
      case ITEM_MUSHROOM: img = aaa_mushroom_10x10; break;
      case ITEM_MUSHROOM_CHOP: img = aaa_mushroom_chopped_10x10; break;
      case ITEM_CARROT: img = aaa_carrot_10x10; break;
      case ITEM_CARROT_CHOP: img = aaa_carrot_chopped_10x10; break;
      default: break;
    }
    if(img) DrawSpriteTransparent(7*TILE+3, 8*TILE+3, img, 10, 10);
  }

  if(StoredItem2 != ITEM_NONE){
    img = 0;
    switch(StoredItem2){
      case ITEM_TOMATO: img = aaa_tomato_10x10; break;
      case ITEM_TOMATO_CHOP: img = aaa_tomato_chopped_10x10; break;
      case ITEM_MUSHROOM: img = aaa_mushroom_10x10; break;
      case ITEM_MUSHROOM_CHOP: img = aaa_mushroom_chopped_10x10; break;
      case ITEM_CARROT: img = aaa_carrot_10x10; break;
      case ITEM_CARROT_CHOP: img = aaa_carrot_chopped_10x10; break;
      default: break;
    }
    if(img) DrawSpriteTransparent(7*TILE+3, 9*TILE+3, img, 10, 10);
  }
}

static void ShowRecipeOverlay(void){
  ST7735_FillRect(8, 16, 112, 112, 0x0000);

  ST7735_SetCursor(3,2);
  ST7735_OutString("RECIPES");

  DrawSpriteTransparent(20, 40, aaa_tomato_chopped_10x10, 10, 10);
  DrawSpriteTransparent(38, 40, aaa_mushroom_10x10, 10, 10);
  DrawSpriteTransparent(72, 36, aaa_purplepot_16x16, 16, 16);

  DrawSpriteTransparent(16, 76, aaa_tomato_chopped_10x10, 10, 10);
  DrawSpriteTransparent(34, 76, aaa_mushroom_chopped_10x10, 10, 10);
  DrawSpriteTransparent(52, 76, aaa_carrot_chopped_10x10, 10, 10);
  DrawSpriteTransparent(84, 72, aaa_yellowpot_16x16, 16, 16);

  ST7735_SetCursor(0,15);
  ST7735_OutString("Black/White: back");

  // start 0.5 second button lock handled by timer ISR
  ButtonLock = 1;
  ButtonTicks = 0;

  while(1){
    uint32_t b = Switch_In();

    if(ButtonLock){
      PrevButtons = b;
      Clock_Delay1ms(20);
      continue;
    }

    if(ReadBlackPressEdge(b) || ReadWhitePressEdge(b)){
      while(Switch_In() & (BLACK_BTN | WHITE_BTN)){
        Clock_Delay1ms(20);
      }
      break;
    }

    PrevButtons = b;
    Clock_Delay1ms(20);
  }

  PrevButtons = 0;
  DrawBoard();
}

static void HandleInteract(void){
  int oc, orow;
  Object_t obj;

  if(!FindAdjacentObject(&oc, &orow, &obj)) return;

  switch(obj){
    case OBJ_NONE:
    break;
    case OBJ_TOMATO:
      HeldItem = ITEM_TOMATO;
      HeldWashed = 0;
      Sound_Collect();
      break;
    case OBJ_MUSHROOM:
      HeldItem = ITEM_MUSHROOM;
      HeldWashed = 0;
      Sound_Collect();
      break;
    case OBJ_CARROT:
      HeldItem = ITEM_CARROT;
      HeldWashed = 0;
      Sound_Collect();
      break;
    case OBJ_POT:
        if(PotReady != 0){
            if(HeldItem == ITEM_NONE){
            CollectPotSoup();
            }
        }else{
            if(HeldItem != ITEM_NONE){
            AddHeldToPot();
            }
        }
    break;
    case OBJ_RECIPE:
      ShowRecipeOverlay();
      break;
    case OBJ_PREP0:
        if(HeldItem != ITEM_NONE){
            ChopSequence();
        }
      break;
    case OBJ_PREP1:
        if(HeldItem != ITEM_NONE && HeldWashed ==0){
            HeldWashed = 1;
            Sound_Splash();
            RedrawCell(ChefCol, ChefRow);
            DrawChef();
            DrawHeldItem();
        }
      break;
    case OBJ_PREP2:
      if(HeldItem != ITEM_NONE && StoredItem1 == ITEM_NONE){
        StoredItem1 = HeldItem;
        StoredWashed1 = HeldWashed;
        HeldItem = ITEM_NONE;
        HeldWashed = 0;
      }else if(HeldItem == ITEM_NONE && StoredItem1 != ITEM_NONE){
        HeldItem = StoredItem1;
        HeldWashed = StoredWashed1;
        StoredItem1 = ITEM_NONE;
        StoredWashed1 = 0;
      }
      break;

    case OBJ_PREP3:
      if(HeldItem != ITEM_NONE && StoredItem2 == ITEM_NONE){
        StoredItem2 = HeldItem;
        StoredWashed2 = HeldWashed;
        HeldItem = ITEM_NONE;
        HeldWashed = 0;
      }else if(HeldItem == ITEM_NONE && StoredItem2 != ITEM_NONE){
        HeldItem = StoredItem2;
        HeldWashed = StoredWashed2;
        StoredItem2 = ITEM_NONE;
        StoredWashed2 = 0;
      }
      break;
      }
  DrawBoard();
}

static void ResetGame(void){
  ChefCol = 3;
  ChefRow = 5;
  ChefFacing = FACE_RIGHT;
  HeldItem = ITEM_NONE;
  HeldWashed = 0;
  Score = 0;
  GameCounter = GAME_TICKS;
  MoveCooldown = 0;
  PotReady = 0;
  PotReadyPoints = 0;
  StoredItem1 = ITEM_NONE;
  StoredItem2 = ITEM_NONE;
  StoredWashed1 = 0;
  StoredWashed2 = 0;
  ClearPot();
}

void Game_Init(void){
  __disable_irq();
  Clock_Init80MHz(0);
  LaunchPad_Init();
  ST7735_InitR(INITR_BLACKTAB);
  ST7735_FillScreen(0x0000);

  Switch_Init();
  ADCinit();
  Sound_Init();
  TimerG0_IntArm(2000, 1000, 2);  // 40MHz/1000/2000 = 20Hz = 50ms per interrupt
  __enable_irq();

  ST7735_DrawBitmap(0, 159, Title_Screen_Image, 128, 160);
}

int Game_Loop(void){
  uint32_t x, y, b;
  int languageSel = 0;

  while(1){
    b = Switch_In();

    switch(State){
      case STATE_TITLE:
        if(ReadBlackPressEdge(b) || ReadWhitePressEdge(b)){
            State = STATE_LANGUAGE;
            DrawLanguageScreen(languageSel);
        }
        Clock_Delay1ms(50);
        break;

      case STATE_LANGUAGE:
        ADCin2(&x,&y);

        if(y < 1000){
          languageSel = 0;   
          DrawLanguageScreen(languageSel);
          Clock_Delay1ms(150);
        }else if(y > 3000){
          languageSel = 1;  
          DrawLanguageScreen(languageSel);
          Clock_Delay1ms(150);
        }

        if(ReadBlackPressEdge(b)){
          Language = (languageSel == 0) ? LANG_EN : LANG_ES;
          ResetGame();
          DrawBoard();
          DrawHomeOverlay();
          State = STATE_HOME;
        }else if(ReadWhitePressEdge(b)){
          State = STATE_TITLE;
          ST7735_DrawBitmap(0, 159, Title_Screen_Image, 128, 160);
        }

        Clock_Delay1ms(50);
        break;

      case STATE_INSTRUCTIONS:
        if(ReadBlackPressEdge(b)){
          ResetGame();
          DrawBoard();
          State = STATE_PLAYING;
        }else if(ReadWhitePressEdge(b)){
          DrawBoard();
          DrawHomeOverlay();
          State = STATE_HOME;
        }
        Clock_Delay1ms(50);
        break;
      case STATE_HOME:
        if(ReadBlackPressEdge(b)){
        DrawInstructionsScreen();
        State = STATE_INSTRUCTIONS;
      }else if(ReadWhitePressEdge(b)){
          State = STATE_LANGUAGE;
          DrawLanguageScreen(languageSel);
        }
        Clock_Delay1ms(50);
        break;
    
      case STATE_PAUSED:
        if(ReadWhitePressEdge(b)){
            DrawBoard();
            State = STATE_PLAYING;
        }else if(ReadBlackPressEdge(b)){
            Sound_GameOver();
            DrawGameOverScreen();
            State = STATE_GAMEOVER;
        }
        Clock_Delay1ms(50);
        break;

      case STATE_PLAYING:
        if(ReadWhitePressEdge(b)){
            DrawPauseOverlay();
            State = STATE_PAUSED;
            break;
        }
        ADCin2(&x,&y);

        if(MoveCooldown) MoveCooldown--;

        if(MoveCooldown == 0){
          if(x > 3000){
            MoveChef(-1,0); 
            MoveCooldown = MOVE_COOLDOWN_TICKS;
          }else if(x < 1000){
            MoveChef(1,0);  
            MoveCooldown = MOVE_COOLDOWN_TICKS;
          }else if(y < 1000){
            MoveChef(0,-1); 
            MoveCooldown = MOVE_COOLDOWN_TICKS;
          }else if(y > 3000){
            MoveChef(0,1);  
            MoveCooldown = MOVE_COOLDOWN_TICKS;
          }
        }

        if(ReadBlackPressEdge(b)){
          HandleInteract();
        }

        DrawHUD();

        Clock_Delay1ms(HOME_TICK_MS);
        if(GameCounter) GameCounter--;
        if(GameCounter == 0){
          Sound_GameOver();
          DrawGameOverScreen();
          State = STATE_GAMEOVER;
        }
        break;

      case STATE_GAMEOVER:
        if(ReadWhitePressEdge(b)){
          ResetGame();
          DrawBoard();
          State = STATE_PLAYING;
        }else if(ReadBlackPressEdge(b)){
          State = STATE_HOME;
          ResetGame();
          DrawBoard();
          DrawHomeOverlay();
        }
        Clock_Delay1ms(50);
        break;

      default:
        State = STATE_TITLE;
        break;
    }

    PrevButtons = b;
  }
}
