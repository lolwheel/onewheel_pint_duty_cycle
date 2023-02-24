#include <stdint.h>

#define SOC_ADDR  0x20001C38
#define VOLTAGES_ADDR 0x20001ABC
#define TRIP_MILLIAMP_HOURS_ADDR 0x20001BD4
#define TRIP_REGEN_ADDR 0x20001BD8
#define RAM_END 0x200002B0 + 0x1998
#define BAT_CAPACITY_MAH 5600

#define clamp(val, low, high) (val < low ? low : val > high ? high : val)


uint16_t batteryCapacityMilliampHours = BAT_CAPACITY_MAH;

uint8_t* socAddress = (uint8_t *)SOC_ADDR;
uint16_t* voltagesAddress = (uint16_t *)VOLTAGES_ADDR;
uint32_t* tripMilliampHoursAddress = (uint32_t *)TRIP_MILLIAMP_HOURS_ADDR;
uint32_t* tripRegenAddress = (uint32_t *)TRIP_REGEN_ADDR;
uint8_t* ramEndAddress = (uint8_t *)RAM_END;


uint8_t openCircuitSocFromCellVoltage(uint16_t cellVoltageMillivolts) {
  static const uint16_t LOOKUP_TABLE_RANGE_MIN_MV = 2700;
  static const uint16_t LOOKUP_TABLE_RANGE_MAX_MV = 4200;
  static uint8_t LOOKUP_TABLE[31] = { 0,  0,  0,  0,  1,  2,  3,  4,  5,  7,  8,
                                     11, 14, 16, 18, 19, 25, 30, 33, 37, 43, 48,
                                     53, 60, 67, 71, 76, 82, 92, 97, 100 };
  static const int16_t LOOKUP_TABLE_SIZE =
      (sizeof(LOOKUP_TABLE) / sizeof(*LOOKUP_TABLE));
  static const int16_t RANGE =
      LOOKUP_TABLE_RANGE_MAX_MV - LOOKUP_TABLE_RANGE_MIN_MV;
  cellVoltageMillivolts =
      clamp(cellVoltageMillivolts - LOOKUP_TABLE_RANGE_MIN_MV, 0, RANGE - 1);
  int32_t lookupIndex = (cellVoltageMillivolts * (LOOKUP_TABLE_SIZE - 1)) / RANGE;
  int32_t leftIndex = lookupIndex;
  int32_t rightIndex = leftIndex + 1;
  int32_t leftValue = LOOKUP_TABLE[leftIndex];
  int32_t rightValue = LOOKUP_TABLE[rightIndex];
  int32_t fractional = (cellVoltageMillivolts * (LOOKUP_TABLE_SIZE - 1)) % RANGE;
  int32_t result = leftValue + (rightValue - leftValue) * fractional / RANGE;
  return clamp(result, 0, 100);
}

uint8_t socFromStoredVoltages() {
  uint16_t foundVoltage = voltagesAddress[0];
  uint16_t lowestVoltage = foundVoltage;

  for (uint8_t i = 1; i < 15; i++) {
    foundVoltage = voltagesAddress[i];

    if (foundVoltage < lowestVoltage) {
      lowestVoltage = foundVoltage;
    }
  }

  return openCircuitSocFromCellVoltage(lowestVoltage); 
}

void deriveAndWriteSoc() {
  if (voltagesAddress[0] == 0) {
    return;
  }

  uint8_t* initialized = ramEndAddress + 1;
  uint8_t* noLoadSoc = ramEndAddress + 2;

  if (*initialized == 0) {
    *noLoadSoc = socFromStoredVoltages();
    *initialized = 1;
  }

  int32_t milliampHoursUsed = (*tripMilliampHoursAddress - *tripRegenAddress) / 20000; // Stored value is 20000x actual value
  int8_t milliampHoursUsedPercent = (milliampHoursUsed * 100) / batteryCapacityMilliampHours;

  *socAddress = clamp(*noLoadSoc - milliampHoursUsedPercent, 0, 100);
}

