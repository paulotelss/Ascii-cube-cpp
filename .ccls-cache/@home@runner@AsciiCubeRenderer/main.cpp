#include <cmath>
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>

#ifdef _WIN32
#include <windows.h>
void usleep(__int64 usec) {
    HANDLE timer;
    LARGE_INTEGER ft;
    ft.QuadPart = -(10 * usec); // Convert to 100 nanosecond interval, negative value indicates relative time
    timer = CreateWaitableTimer(NULL, TRUE, NULL);
    SetWaitableTimer(timer, &ft, 0, NULL, NULL, 0);
    WaitForSingleObject(timer, INFINITE);
    CloseHandle(timer);
}
#else
#include <unistd.h>
#endif

float A, B, C;
const int width = 160, height = 44;
const int distanceFromCam = 100;
const float K1 = 40, incrementSpeed = 0.6;
std::vector<float> zBuffer(width * height);
std::vector<char> buffer(width * height, '.');

float calculateX(int i, int j, int k) {
    return j * std::sin(A) * std::sin(B) * std::cos(C) - k * std::cos(A) * std::sin(B) * std::cos(C) +
           j * std::cos(A) * std::sin(C) + k * std::sin(A) * std::sin(C) + i * std::cos(B) * std::cos(C);
}

float calculateY(int i, int j, int k) {
    return j * std::cos(A) * std::cos(C) + k * std::sin(A) * std::cos(C) -
           j * std::sin(A) * std::sin(B) * std::sin(C) + k * std::cos(A) * std::sin(B) * std::sin(C) -
           i * std::cos(B) * std::sin(C);
}

float calculateZ(int i, int j, int k) {
    return k * std::cos(A) * std::cos(B) - j * std::sin(A) * std::cos(B) + i * std::sin(B);
}

void calculateForSurface(float cubeX, float cubeY, float cubeZ, char ch, float horizontalOffset) {
    float x = calculateX(cubeX, cubeY, cubeZ);
    float y = calculateY(cubeX, cubeY, cubeZ);
    float z = calculateZ(cubeX, cubeY, cubeZ) + distanceFromCam;
    float ooz = 1 / z;
    int xp = static_cast<int>(width / 2 + horizontalOffset + K1 * ooz * x * 2);
    int yp = static_cast<int>(height / 2 + K1 * ooz * y);
    int idx = xp + yp * width;
    if (idx >= 0 && idx < width * height && ooz > zBuffer[idx]) {
        zBuffer[idx] = ooz;
        buffer[idx] = ch;
    }
}

void renderCube(float cubeWidth, float horizontalOffset) {
    for (float cubeX = -cubeWidth; cubeX < cubeWidth; cubeX += incrementSpeed) {
        for (float cubeY = -cubeWidth; cubeY < cubeWidth; cubeY += incrementSpeed) {
            calculateForSurface(cubeX, cubeY, -cubeWidth, '@', horizontalOffset);
            calculateForSurface(cubeWidth, cubeY, cubeX, '$', horizontalOffset);
            calculateForSurface(-cubeWidth, cubeY, -cubeX, '~', horizontalOffset);
            calculateForSurface(-cubeX, cubeY, cubeWidth, '#', horizontalOffset);
            calculateForSurface(cubeX, -cubeWidth, -cubeY, ';', horizontalOffset);
            calculateForSurface(cubeX, cubeWidth, cubeY, '+', horizontalOffset);
        }
    }
}

int main() {
    std::cout << "\x1b[2J";
    while (true) {
        std::fill(buffer.begin(), buffer.end(), '.');
        std::fill(zBuffer.begin(), zBuffer.end(), 0);
        renderCube(20, -40); // Primeiro cubo
        renderCube(10, 10);  // Segundo cubo
        renderCube(5, 40);   // Terceiro cubo
        std::cout << "\x1b[H";
        for (int k = 0; k < width * height; k++) {
            std::cout << buffer[k];
            if (k % width == width - 1)
                std::cout << '\n';
        }
        A += 0.05;
        B += 0.05;
        C += 0.01;
        std::this_thread::sleep_for(std::chrono::microseconds(16000));
    }
    return 0;
}
