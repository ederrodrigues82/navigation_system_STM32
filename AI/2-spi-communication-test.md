1- Implement a function to test the communication between Raspi and STM32
2- Populate / mock the lawn_mower_status to test the communication
3- Implement the Raspi python application to communicate with STM32 using SPI full duplex (STM32 is master)
4- The Raspi application will implement the high level lawn mower movement, so use Oriented Object and good practices because this application will growth
5- Debug Considerations:
    a) The STM32 doesn't have a gdb option, only string test
    b) The Raspi has debug so develop the application considering that we have more debug resources on the Raspi side.
    c) On the Raspi side implement a menu that help to debug some feature on the STM32 side.
6- At this moment focus only on communication test considering the metioned bollut points.