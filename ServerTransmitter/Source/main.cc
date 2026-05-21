#include "ServerTransmitter.h"


int main() {
    std::shared_ptr<Camera> cameraInst = Camera::getInstance("video0");
    std::shared_ptr<Blurring> blurringInst = std::make_shared<Blurring>(cameraInst);

    std::unique_ptr<ServerTransmitter> transmitterInst = std::make_unique<ServerTransmitter>(blurringInst);
    transmitterInst->startTransmitFrame();

    while(;;) {
        ;
    }
    return 0;
}