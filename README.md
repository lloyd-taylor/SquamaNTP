# SquamaNTP

NTP Clock based on SeeedStudio's Squama Ethernet board with PoE: 
    https://www.seeedstudio.com/Squame-Ethernet-Arduino-W5500-Ethernet-Board-with-PoE-p-4822.html
and SeedStudio's Grove 0.54" Red Quad Alphanumeric Display
    https://www.seeedstudio.com/Grove-0-54-Red-Quad-Alphanumeric-Display-p-4032.html

Wiring is straightforward: Plug Squama Board into Display using Grove Cable.  Plug Squama Board into PoE Ethernet Switch Port.

IMPORTANT: Do NOT connect USB power to board if ethernet is plugged into a PoE switchport.  Unexpected results (incuding frying your computer or the Squama Board) may result with two separate power feeds!

Either plug the Squama board into a NON-PoE Port on your switch during development, or use a power-switched USB hub between your computer and the Squama board. 
