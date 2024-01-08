

#ifndef _SETTINGS_HPP_
#define _SETTINGS_HPP_

class Settings
{
    public:
        Settings();
        void setup();
        /**
         * @brief Get the Percentage of the battery
         * 
         * @return int Percentage of the battery 
         */

        /**
         * @brief Function to update the battery status. This should be called on a regular basis 
         * 
         */
        void update();
    private:
};

#endif