/** LiquidCrystal pins Defines */
#define contrast 3
#define valueofContrast 120 //0-255
#define rs 12
#define en 11
#define d4 2
#define d5 4
#define d6 5
#define d7 6
#define cols 20
#define rows 4

/** Input Pullups Buttons Defines */
#define drch 8
#define izq 7

/** Relays pins defines */
#define top 10
#define bot 9

/** Sensor PAS parameters */
#define sensorPin A3
#define cadenceMagnets 12.0
#define interval 1000 //ms
#define maxRPM 120
#define minRPM 0
#define _Chainring 42
#define _Cogset 11

/** Interval getvalues */
#define intervalValues 500



/** 
 *  LCD DISTRIBUTION (16x4)
 *      0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  16  17  18  19
 * 0    Cm  :   0   0   .   0   A                               Bp  :   1   0   0   %   
 * 1                                V   :   0   0   Kh      Tm  :   0   0   .   0   C
 * 2    Cb  :   0   0   .   0   A                           Dt  :   0   0   .   0   Km
 * 3    Dc  :   0   0   .   0   %       -   -   -   -   -   -   -   -   -   -   -   -

To organice parameters inside LCD, change values of defines. Each value indicate the first
position of each parameters in coloums.
*/

#define _motorCurrent 0
#define _batteryPercentage 14
#define _velocity 7
#define _mosfetTem 13
#define _battCurrent 0
#define _distance 13
#define _dutyCicle 0