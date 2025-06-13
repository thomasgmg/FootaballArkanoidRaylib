#include "raylib.h"
#include "raymath.h"
#include <cstdio>

struct Ball
{
    Vector2 Position;
    float Radius;
    float Speed;
    Vector2 Direction;
    Color BallColor;
    enum
    {
        NORMAL,
        ROLLING
    } State;
    float RollTimer;
    float rollDirection;
    float spinAngle;
    float spinSpeed;
};
struct Ball ball = {.Position = {0.5f, 0.5f},
                    .Radius = 0.008f,
                    .Speed = 0.42f,
                    .Direction = {1.0f, -1.0f},
                    .BallColor = WHITE,
                    .State = Ball::NORMAL,
                    .RollTimer = 0.0f,
                    .rollDirection = 0.0f,
                    .spinAngle = 0.0f,
                    .spinSpeed = 360.0f};

struct Goalkeeper
{
    Vector2 Position;
    float Width;
    float Height;
    float Speed;
    Color KeeperColor;
};
struct Goalkeeper keeper = {
    .Position = {0.96f, 0.5f}, .Width = 0.016f, .Height = 0.056f, .Speed = 0.485f, .KeeperColor = DARKBLUE};

struct Goal
{
    Vector2 Position;
    float Width;
    float Height;
    Color GoalColor;
    Color NetColor;
};
struct Goal goal = {
    .Position = {0.992f, 0.5f}, .Width = 0.024f, .Height = 0.308f, .GoalColor = GRAY, .NetColor = WHITE};

struct Particle
{
    Vector2 position;
    Vector2 velocity;
    Color color;
    float alpha;
    float size;
    float life;
};
int const MAX_PARTICLES = 100;
Particle particles[MAX_PARTICLES];
int particleCount = 0;

int score = 0;
int goals = 0;
bool Pause = false;

// Declaration
void DrawGame(void);
void DrawFootballField();
void DrawFootballBall(Vector2 position, float radius);
void DrawGoalkeeper(Vector2 position, float width, float height);
void DrawGoal(Vector2 position, float width, float height);
void UpdateGame(float deltaTime);
void BallWallCollision(void);
void BallGoalkeeperCollision(void);
void BallGoalCollision(void);
void CreateGoalEffect(Vector2 goalPos);
void UpdateDrawParticles(float deltaTime);

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(1250, 650, "Classic Game: Football Arkanoid");
    SetTargetFPS(60);

    // Normalize ball direction
    ball.Direction = Vector2Normalize(ball.Direction);

    while (!WindowShouldClose())
    {
        float deltaTime = GetFrameTime();

        if (IsKeyPressed(KEY_SPACE))
        {
            Pause = !Pause;
        }

        UpdateGame(deltaTime);
        BallWallCollision();
        BallGoalkeeperCollision();
        BallGoalCollision();

        DrawGame();
    }

    CloseWindow();
    return 0;
}

void UpdateGame(float deltaTime)
{
    if (Pause)
    {
        return;
    }

    // Update Goalkeeper
    Vector2 keeperPixelPos = {keeper.Position.x * GetScreenWidth(), keeper.Position.y * GetScreenHeight()};
    if (IsKeyDown(KEY_UP) && keeperPixelPos.y - keeper.Height * GetScreenHeight() / 2 > 0)
    {
        keeper.Position.y -= keeper.Speed * deltaTime;
    }
    if (IsKeyDown(KEY_DOWN) && keeperPixelPos.y + keeper.Height * GetScreenHeight() / 2 < GetScreenHeight())
    {
        keeper.Position.y += keeper.Speed * deltaTime;
    }

    // Update ball
    if (ball.State == Ball::NORMAL)
    {
        ball.Position.x += ball.Direction.x * ball.Speed * deltaTime;
        ball.Position.y += ball.Direction.y * ball.Speed * deltaTime;
        ball.spinAngle = 0.0f;
    }

    else if (ball.State == Ball::ROLLING)
    {
        ball.RollTimer -= deltaTime;
        float reducedSpeed = ball.Speed * 0.2f;
        float newY = ball.Position.y + ball.rollDirection * reducedSpeed * deltaTime;

        float goalTop = goal.Position.y - goal.Height / 2 + ball.Radius;
        float goalBottom = goal.Position.y + goal.Height / 2 - ball.Radius;
        if (newY < goalTop)
        {
            newY = goalTop;
            ball.rollDirection = 1.0f; // Reverse direction to move down
        }
        else if (newY > goalBottom)
        {
            newY = goalBottom;
            ball.rollDirection = -1.0f; // Reverse direction to move up
        }
        ball.Position.y = newY;

        // Keep ball at goal's x-position
        ball.Position.x = goal.Position.x - ball.Radius;

        ball.spinAngle += ball.spinSpeed * deltaTime;

        if (ball.spinAngle >= 360.0f)
        {
            ball.spinAngle -= 360.0f;
        }

        if (ball.RollTimer <= 0.0f)
        {
            // Reset ball to starting position
            ball.Position = (Vector2){0.5f, 0.5f};
            ball.Direction = (Vector2){-1.0f, (float)GetRandomValue(-100, 100) / 100.0f};
            ball.Direction = Vector2Normalize(ball.Direction);
            ball.State = Ball::NORMAL;
        }
    }
}

void CreateGoalEffect(Vector2 goalPos)
{
    int particlesPerBlock = 4;
    for (int i = 0; i < 8; i++)
    {
        for (int p = 0; p < particlesPerBlock && particleCount < MAX_PARTICLES; p++)
        {
            Particle &particle = particles[particleCount];
            particle.position = {goalPos.x * GetScreenWidth(), goalPos.y * GetScreenHeight()};
            particle.velocity = {(float)(GetRandomValue(-200, 200)) / 100.0f,
                                 (float)(GetRandomValue(-200, 200)) / 100.0f};
            particle.color = MAROON;
            particle.alpha = 1.0f;
            particle.size = (float)GetRandomValue(5, 20);
            particle.life = 0.5f + GetRandomValue(0, 150) / 100.0f;
            particleCount++;
        }
    }
}

void UpdateDrawParticles(float deltaTime)
{
    for (int i = particleCount - 1; i >= 0; i--)
    {
        Particle &p = particles[i];
        p.position.x += p.velocity.x * deltaTime * 120;
        p.position.y += p.velocity.y * deltaTime * 120;
        p.life -= deltaTime;
        p.alpha = p.life / 1.5f;

        Color particleColor = p.color;
        particleColor.a = (unsigned char)(p.alpha * 255);
        DrawRectanglePro({p.position.x, p.position.y, p.size, p.size}, {p.size / 2, p.size / 2}, GetTime() * 90,
                         particleColor);

        if (p.life <= 0)
        {
            particles[i] = particles[particleCount - 1];
            particleCount--;
        }
    }
}

void BallWallCollision(void)
{
    // Ball wall collision
    Vector2 ballPixelPos = {ball.Position.x * GetScreenWidth(), ball.Position.y * GetScreenHeight()};
    float ballRadiusPixels = ball.Radius * GetScreenWidth();
    if (ballPixelPos.y + ballRadiusPixels >= GetScreenHeight() || ballPixelPos.y - ballRadiusPixels <= 0)
    {
        ball.Direction.y = -ball.Direction.y;
        ball.Position.y = (ballPixelPos.y + ballRadiusPixels >= GetScreenHeight() ? GetScreenHeight() - ballRadiusPixels
                                                                                  : ballRadiusPixels) /
                          GetScreenHeight();
    }
    if (ballPixelPos.x - ballRadiusPixels <= 0)
    {
        ball.Direction.x = -ball.Direction.x;
        ball.Position.x = ballRadiusPixels / GetScreenWidth();
    }

    if (ballPixelPos.x + ballRadiusPixels >= GetScreenWidth())
    {
        ball.Position = (Vector2){0.5f, 0.5f};
        ball.Direction = (Vector2){-1.0f, (float)GetRandomValue(-100, 100) / 100.0f};
        ball.Direction = Vector2Normalize(ball.Direction);
    }
}

void BallGoalkeeperCollision(void)
{
    // Ball-goalkeeper collision
    Vector2 keeperPixelPos = {keeper.Position.x * GetScreenWidth(), keeper.Position.y * GetScreenHeight()};
    Rectangle keeperRect = {keeperPixelPos.x - keeper.Width * GetScreenWidth() / 2,
                            keeperPixelPos.y - keeper.Height * GetScreenHeight() / 2, keeper.Width * GetScreenWidth(),
                            keeper.Height * GetScreenHeight()};
    Vector2 ballPixelPos = {ball.Position.x * GetScreenWidth(), ball.Position.y * GetScreenHeight()};
    if (CheckCollisionCircleRec(ballPixelPos, ball.Radius * GetScreenWidth(), keeperRect))
    {
        ball.Direction.x = -ball.Direction.x;
        ball.Position.x = (keeperPixelPos.x - keeper.Width * GetScreenWidth() / 2 - ball.Radius * GetScreenWidth()) /
                          GetScreenWidth();
        score += 100;
    }
}

void BallGoalCollision(void)
{
    // Ball-goal collision
    Vector2 goalPixelPos = {goal.Position.x * GetScreenWidth(), goal.Position.y * GetScreenHeight()};
    Rectangle goalRect = {goalPixelPos.x - goal.Width * GetScreenWidth(),
                          goalPixelPos.y - goal.Height * GetScreenHeight() / 2, goal.Width * GetScreenWidth(),
                          goal.Height * GetScreenHeight()};
    Vector2 ballPixelPos = {ball.Position.x * GetScreenWidth(), ball.Position.y * GetScreenHeight()};
    if (ball.State == Ball::NORMAL && CheckCollisionCircleRec(ballPixelPos, ball.Radius * GetScreenWidth(), goalRect))
    {
        goals++;
        CreateGoalEffect(goal.Position);

        ball.Position.x = goal.Position.x - ball.Radius;
        ball.Position.y = goal.Position.y;
        ball.State = Ball::ROLLING;
        ball.RollTimer = 4.0f;
        ball.rollDirection = (ball.Direction.y >= 0) ? 1.0f : -1.0f;
    }
}

void DrawFootballField()
{
    // Green pitch
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), (Color){0, 100, 0, 255});

    // Center line
    DrawRectangle(GetScreenWidth() / 2 - 2, 10, 1, GetScreenHeight() - 22, WHITE);

    // Center circle
    DrawCircleLines(GetScreenWidth() / 2, GetScreenHeight() / 2, GetScreenWidth() * 0.072f, WHITE);

    // Penalty area
    float penaltyWidth = GetScreenWidth() * 0.144f;
    float penaltyHeight = goal.Height * GetScreenHeight() + GetScreenHeight() * 0.185f;
    DrawRectangleLines(GetScreenWidth() - penaltyWidth - goal.Width * GetScreenWidth() + GetScreenWidth() * 0.016f,
                       (float)GetScreenHeight() / 2 - penaltyHeight / 2, penaltyWidth, penaltyHeight, WHITE);

    // Outer boundary
    DrawRectangleLines(GetScreenWidth() * 0.008f, GetScreenHeight() * 0.015f,
                       GetScreenWidth() - GetScreenWidth() * 0.016f, GetScreenHeight() - GetScreenHeight() * 0.031f,
                       WHITE);
}

void DrawFootballBall(Vector2 position, float radius)
{
    Vector2 pixelPos = {position.x * GetScreenWidth(), position.y * GetScreenHeight()};
    DrawCircleV(pixelPos, radius * GetScreenWidth(), ball.BallColor);
    for (int i = 0; i < 5; i++)
    {
        float angle = (i * (360.0f / 5) + ball.spinAngle) * DEG2RAD;
        Vector2 pentagonPos = {pixelPos.x + cosf(angle) * radius * GetScreenWidth() * 0.5f,
                               pixelPos.y + sinf(angle) * radius * GetScreenWidth() * 0.5f};
        DrawCircleV(pentagonPos, radius * GetScreenWidth() * 0.3f, BLACK);
    }
}

void DrawGoalkeeper(Vector2 position, float width, float height)
{
    // Body
    Vector2 pixelPos = {position.x * GetScreenWidth(), position.y * GetScreenHeight()};
    DrawRectangle(pixelPos.x - width * GetScreenWidth() / 2 - GetScreenWidth() * 0.016f,
                  pixelPos.y - height * GetScreenHeight() / 5, width * GetScreenWidth(), height * GetScreenHeight(),
                  keeper.KeeperColor);
}

void DrawGoal(Vector2 position, float width, float height)
{
    // Goal posts
    Vector2 pixelPos = {position.x * GetScreenWidth(), position.y * GetScreenHeight()};
    DrawRectangle(pixelPos.x - width * GetScreenWidth(), pixelPos.y - height * GetScreenHeight() / 2,
                  width * GetScreenWidth(), height * GetScreenHeight(), goal.GoalColor);

    // Net (grid of lines)
    int netLines = 7;
    float netSpacingX = width * GetScreenWidth() / netLines;
    float netSpacingY = height * GetScreenHeight() / netLines;
    for (int i = 1; i < netLines; i++)
    {
        DrawLine(pixelPos.x - width * GetScreenWidth() + i * netSpacingX, pixelPos.y - height * GetScreenHeight() / 2,
                 pixelPos.x - width * GetScreenWidth() + i * netSpacingX, pixelPos.y + height * GetScreenHeight() / 2,
                 goal.NetColor);
        DrawLine(pixelPos.x - width * GetScreenWidth(), pixelPos.y - height * GetScreenHeight() / 2 + i * netSpacingY,
                 pixelPos.x, pixelPos.y - height * GetScreenHeight() / 2 + i * netSpacingY, goal.NetColor);
    }
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(BLACK);

    DrawFootballField();
    DrawGoal(goal.Position, goal.Width, goal.Height);
    DrawGoalkeeper(keeper.Position, keeper.Width, keeper.Height);
    DrawFootballBall(ball.Position, ball.Radius);
    UpdateDrawParticles(GetFrameTime());

    DrawText(TextFormat("Score: %i", score), GetScreenWidth() * 0.008f, GetScreenHeight() * 0.015f, 20, WHITE);
    DrawText(TextFormat("Goals: %i", goals), GetScreenWidth() * 0.008f, GetScreenHeight() * 0.062f, 20, WHITE);

    if (Pause)
    {
        DrawText("Game paused", GetScreenWidth() / 2.0f - MeasureText("Game paused", 25) / 2.0f,
                 GetScreenHeight() / 2.0f, 25, BLACK);
    }

    if (ball.State == Ball::ROLLING)
    {
            DrawText("Goal", (float)GetScreenWidth() / 2 + 250, (float)GetScreenHeight() / 2, 30, BLACK);
    }

    EndDrawing();
}
