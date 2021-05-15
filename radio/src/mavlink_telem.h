/*
 * (c) www.olliw.eu, OlliW, OlliW42
 */

#define MAVLINK_RAM_SECTION  __attribute__((section (".ram")))

// -- CoOS RTOS mavlink task handlers --

void mavlinkStart();
uint16_t mavlinkTaskRunTime(void);
uint16_t mavlinkTaskRunTimeMax(void);
uint16_t mavlinkTaskLoop(void);


// -- main Mavlink stuff --

#define MAVLINK_TELEM_MY_SYSID        254 //MissionPlanner is 255, QGroundControl is 255
#define MAVLINK_TELEM_MY_COMPID       (MAV_COMP_ID_MISSIONPLANNER + 4) //191 is companion, 194 is free

// tick10ms() is called every 10 ms from 10ms ISR
// if this is changed, timing needs to be adapted !!
#define MAVLINK_TELEM_RECEIVING_TIMEOUT                 330 // 3.3 secs
#define MAVLINK_TELEM_RADIO_RECEIVING_TIMEOUT           330 // 3.3 secs
#define MAVLINK_TELEM_GIMBALMANAGER_RECEIVING_TIMEOUT   330 // 3.3 secs ATTENTION: a GM may emit at slow rate

//COMMENT:
//  except of where noted, functions/structs use units of the MAVLink message
//  the mavsdk caller/setter functions however use native units, and deg, whenever possible

class MavlinkTelem
{
  public:
    MavlinkTelem() { _init(); } // constructor

    void wakeup();
    void tick10ms();

    // MAVSDK GENERAL

    bool telemetryVoiceEnabled(void);


  // PROTECTED FIELDS and METHODS
  protected:

    void _init(void);
};

extern MavlinkTelem mavlinkTelem;
