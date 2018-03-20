#include <Arduino.h>
#include "pins.h"
#include "random.h"
#include "input.h"
#include "games.h"
#include "players.h"
#include "fighters.h"

enum Screen
{
  RandomizerInit = 0,
  GameSelection = 1,
  PlayerSelection = 2,
  FighterSelection = 3,
};

bool doUpdate = true;
bool commandAwait = true;

int screen_state = Screen::RandomizerInit;

bool redraw_needed = true;

void setup()
{
  Serial.begin(9600);

  lcd.begin(16, 2);
  pinMode(pin_led_green, OUTPUT);
  pinMode(pin_led_blue, OUTPUT);
  pinMode(pin_button_black, INPUT_PULLUP);
  pinMode(pin_button_joystick, INPUT_PULLUP);

  initRandom();

  // For testing
  //screen_state = Screen::GameSelection;
}

void RandomizerInitScreen()
{
  if (redraw_needed)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Randomizer ");
    lcd.setCursor(0, 1);
    lcd.print("S ");
    lcd.print(random_seed);
    lcd.print(" F ");
    lcd.print(random_fairness);

    Serial.print("Seed: ");
    Serial.println(random_seed);
    Serial.print("Fairness: ");
    Serial.println(random_fairness);

    redraw_needed = false;
  }

  if (input_allowed)
  {
    if (x_right)
    {
      random_fairness = random_fairness + n_players;
      input_allowed = false;
      redraw_needed = true;
    }

    if (x_left)
    {
      random_fairness = random_fairness - n_players;
      input_allowed = false;
      redraw_needed = true;
    }

    if (button_black_pressed)
    {
      screen_state = Screen::GameSelection;
      Serial.println("Go to GameSelection");

      input_allowed = false;
      redraw_needed = true;
    }
  }
}

void GameSelectionScreen()
{
  if (redraw_needed)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(games[games_index]);
    Serial.print("Game: ");
    Serial.println(games[games_index]);

    redraw_needed = false;
  }

  if (input_allowed)
  {
    if (x_right)
    {
      games_index = (games_index + n_games + 1) % n_games;
      fighter_map_selected = fighter_map[games_index];
      n_fighter_map_selected = n_fighter_map[games_index];
      input_allowed = false;
      redraw_needed = true;
    }

    if (x_left)
    {
      games_index = (games_index + n_games - 1) % n_games;
      fighter_map_selected = fighter_map[games_index];
      n_fighter_map_selected = n_fighter_map[games_index];
      input_allowed = false;
      redraw_needed = true;
    }

    if (button_black_pressed)
    {
      screen_state = Screen::PlayerSelection;
      Serial.println("Go to PlayerSelection");

      input_allowed = false;
      redraw_needed = true;
    }
  }
}

void PlayerSelectionScreen()
{
  player_index_first = 0;
  player_index_second = 1;
  screen_state = Screen::FighterSelection;
  Serial.println("Go to FighterSelection");
}

enum Winner
{
  None,
  First,
  Second,
  Draw,
};

int winner_selected = Winner::None;
bool not_fair_win = false;

void FighterSelectionScreen()
{
  if (redraw_needed)
  {
    fighter_index_first = random(n_fighter_map_selected);
    fighter_index_second = random(n_fighter_map_selected);

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(fighter_map_selected[fighter_index_first]);
    lcd.setCursor(0, 1);
    lcd.print(fighter_map_selected[fighter_index_second]);

    Serial.print("First: ");
    Serial.println(fighter_map_selected[fighter_index_first]);
    Serial.print("Second: ");
    Serial.println(fighter_map_selected[fighter_index_second]);

    redraw_needed = false;
  }

  if (input_allowed)
  {
    if (button_black_pressed)
    {
      screen_state = Screen::GameSelection;
      Serial.println("Go to GameSelection");

      input_allowed = false;
      redraw_needed = true;
    }
  }

  if (x_left)
  {
    winner_selected = Winner::First;
  }

  if (x_right)
  {
    winner_selected = Winner::Second;
  }

  if (!x_left && !x_right && y_up)
  {
    winner_selected = Winner::Draw;
  }

  if (y_down || (x_left && y_up) || (x_right && y_up) || (x_left && x_right))
  {
    winner_selected = Winner::None;
  }

  not_fair_win = y_down;

  digitalWrite(pin_led_green, x_left || x_right);
  digitalWrite(pin_led_blue, y_down || y_up);

  if (winner_selected != Winner::None && x_center && y_center)
  {
    Serial.print("Won: ");
    Serial.println(winner_selected);
    Serial.print("Fair: ");
    Serial.println(!not_fair_win);

    screen_state = Screen::FighterSelection;
    Serial.println("Go to FighterSelection");

    winner_selected = Winner::None;
    input_allowed = false;
    redraw_needed = true;
  }
}

void loop()
{
  readInput();

  switch (screen_state)
  {
  case Screen::RandomizerInit:
    RandomizerInitScreen();
    break;

  case Screen::GameSelection:
    GameSelectionScreen();
    break;

  case Screen::PlayerSelection:
    PlayerSelectionScreen();
    break;

  case Screen::FighterSelection:
    FighterSelectionScreen();
    break;
  }
}
