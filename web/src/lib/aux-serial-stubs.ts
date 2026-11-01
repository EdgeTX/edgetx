// Modules of radios with an aux serial port import these to bridge it to a host
// serial port. The page has nothing to connect them to, so the firmware sees a
// port on which nothing ever arrives. They are required regardless: instantiating
// a module fails if any of its imports is missing.
export const auxSerialStubs = {
  simuAuxSerialStart: (): void => {},
  simuAuxSerialStop: (): void => {},
  simuAuxSerialSetBaudrate: (): void => {},
  simuAuxSerialSendBuffer: (): void => {},
};
