#pragma once

class TransmitterIf {
    public:
        virtual bool startTransmitFrame();
        virtual bool stopTransmitFrame(); 
};