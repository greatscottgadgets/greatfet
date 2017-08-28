/*
 * This file is part of GreatFET
 */

 #ifndef __DEBUG_H__
 #define __DEBUG_H__

/* Initialize UART2 for debug logging */
void debug_init(void);

/* Log text to UART2 */
void debug_log(char *str);

#endif //__DEBUG_H__
