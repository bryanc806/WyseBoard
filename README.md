# WyseBoard

This is a simple converter to allow use of a PS/2 style keyboard with a Wyse 60 serial terminal.

## Connections on an Arduino Pro Mini:

    Pro-Mini Pin	Connection
    ------------    ----------
    3 --> PS/2 Clock
    5 --> PS/2 Data
    9 --> Pro-Mini 10 (SPI SS)
    12 --> Wyse Data (Pin 1)
    13 --> Wyse Clock (Pin 3)
    +5v --> Wyse +5v (Pin 2), PS/2 +5v
    GND --> Wyse Gnd (Pin 4), PS/2 GND


## Operation

The converter uses the SPI interface in slave mode to shift keyboard
data out.  The Wyse protocol is a simple bit array that is shifted into
the terminal while the terminal sends the Clock signal.

To ensure syncronization, the SPI interface is reset if the Clock signal is low for a number of calls to loop().

See https://terminals-wiki.org/wiki/index.php/Wyse_WY-60

The PS2Advanced Library (https://github.com/techpaul/PS2KeyAdvanced) is needed for the PS2 side of things.




