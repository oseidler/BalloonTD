#include "Engine/Core/EngineCommon.hpp"
#pragma once


//forward declarations
struct Vec2;
struct Rgba8;
class App;
class Renderer;
class InputSystem;
class AudioSystem;
class Window;
class RandomNumberGenerator;
class Game;

//external declarations
extern App* g_theApp;
extern Renderer* g_theRenderer;
extern InputSystem* g_theInput;
extern AudioSystem* g_theAudio;
extern Window* g_theWindow;
extern Game* g_theGame;

extern RandomNumberGenerator g_rng;

//gameplay constants
constexpr float SCREEN_CAMERA_SIZE_X = 1600.f;
constexpr float SCREEN_CAMERA_SIZE_Y = 900.f;
constexpr float SCREEN_CAMERA_CENTER_X = SCREEN_CAMERA_SIZE_X / 2.f;
constexpr float SCREEN_CAMERA_CENTER_Y = SCREEN_CAMERA_SIZE_Y / 2.f;

constexpr int   NUM_CURVE_SUBDIVISIONS = 64;
constexpr float CHILD_SPACING = 20.0f;
constexpr float SPEED_MODIFIER = 40.0f;
constexpr float SIZE_MODIFIER = 24.0f;
constexpr float TRACK_WIDTH = 6.0f;

//debug drawing functions
void DebugDrawLine(Vec2 const& startPosition, Vec2 const& endPosition, float width, Rgba8 const& color);
void DebugDrawRing(Vec2 const& center, float radius, float width, Rgba8 const& color);