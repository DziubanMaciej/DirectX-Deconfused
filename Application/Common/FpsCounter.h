#pragma once

template <unsigned int bufferSize>
class FpsCounter {
public:
    void push(unsigned int deltaTimeMicroseconds) {
        currentFrameWrappedIndex = (currentFrameWrappedIndex == bufferSize - 1) ? 0 : currentFrameWrappedIndex + 1;
        if (currentFrameIndex >= bufferSize) {
            // If a ring buffer wrapped at least once, we have to subtract the element being overwritten from sum
            currentFrameTimesSum -= buffer[currentFrameWrappedIndex];
        }

        buffer[currentFrameWrappedIndex] = deltaTimeMicroseconds;
        currentFrameIndex++;
        currentFrameTimesSum += deltaTimeMicroseconds;
    }

    unsigned int getFps() const {
        return bufferSize * 1000000 / currentFrameTimesSum;
    }

    unsigned int getFrameIndex() const {
        return currentFrameIndex;
    }

private:
    unsigned int buffer[bufferSize] = {};                   // ring buffer contains frame durations of subsequent frames
    unsigned int currentFrameIndex = 0u;                    // absolute frame index
    unsigned int currentFrameWrappedIndex = bufferSize - 1; // value of currentFrameIndex % bufferSize cached for performance
    unsigned int currentFrameTimesSum = 0u;                 // sum of all elements of buffer cached for performance
};
