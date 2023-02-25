#include <stdint.h>

template <typename T>
T clamp(T val, T low, T high) {
  return (val < low ? low : val > high ? high : val);
}

struct BSS_T {
  uint8_t initialized;
  uint8_t noLoadSoc;
};

/**
 * Static and non-const seems to be the magic combination to keep thes variables unstripped
 * while still allocated in flash.
 */
static uint32_t BSS_REQUESTED_SIZE
    __attribute__((section(".bss_requested_len"))) __attribute__((__used__)) =
        sizeof(BSS_T);
static BSS_T* BSS __attribute__((section(".bss_ptr")))
__attribute__((__used__)) = (BSS_T*)(0x200002B0 + 0x1998);

constexpr int batteryCapacityMilliampHours = 5600;

uint8_t* const socAddress = (uint8_t*)0x20001C38;
const uint16_t* voltagesAddress = (uint16_t*)0x20001ABC;
const uint32_t* tripMilliampHoursAddress = (uint32_t*)0x20001BD4;
const uint32_t* tripRegenAddress = (uint32_t*)0x20001BD8;

uint8_t openCircuitSocFromCellVoltage(uint16_t cellVoltageMillivolts) {
  constexpr uint16_t LOOKUP_TABLE_RANGE_MIN_MV = 2700;
  constexpr uint16_t LOOKUP_TABLE_RANGE_MAX_MV = 4200;
  static const uint8_t LOOKUP_TABLE[31] = {
      0,  0,  0,  0,  1,  2,  3,  4,  5,  7,  8,  11, 14, 16, 18, 19,
      25, 30, 33, 37, 43, 48, 53, 60, 67, 71, 76, 82, 92, 97, 100};
  constexpr int16_t LOOKUP_TABLE_SIZE =
      (sizeof(LOOKUP_TABLE) / sizeof(*LOOKUP_TABLE));
  constexpr int16_t RANGE =
      LOOKUP_TABLE_RANGE_MAX_MV - LOOKUP_TABLE_RANGE_MIN_MV;
  cellVoltageMillivolts =
      clamp(cellVoltageMillivolts - LOOKUP_TABLE_RANGE_MIN_MV, 0, RANGE - 1);
  const int32_t lookupIndex =
      (cellVoltageMillivolts * (LOOKUP_TABLE_SIZE - 1)) / RANGE;
  const int32_t leftIndex = lookupIndex;
  const int32_t rightIndex = leftIndex + 1;
  const int32_t leftValue = LOOKUP_TABLE[leftIndex];
  const int32_t rightValue = LOOKUP_TABLE[rightIndex];
  const int32_t fractional =
      (cellVoltageMillivolts * (LOOKUP_TABLE_SIZE - 1)) % RANGE;
  const int32_t result =
      leftValue + (rightValue - leftValue) * fractional / RANGE;
  return clamp<int32_t>(result, 0, 100);
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

 void __attribute__((section(".entry"))) __attribute__((__used__)) deriveAndWriteSoc() {
  if (voltagesAddress[0] == 0) {
    return;
  }

  if (BSS->initialized == 0) {
    BSS->noLoadSoc = socFromStoredVoltages();
    BSS->initialized = 1;
  }

  int32_t milliampHoursUsed = (*tripMilliampHoursAddress - *tripRegenAddress) /
                              20000;  // Stored value is 20000x actual value
  int8_t milliampHoursUsedPercent =
      (milliampHoursUsed * 100) / batteryCapacityMilliampHours;

  *socAddress = clamp(BSS->noLoadSoc - milliampHoursUsedPercent, 0, 100);
}
