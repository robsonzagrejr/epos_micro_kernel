// EPOS EPOSMote III (ARM Cortex-M3) SETUP

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    // LSB must be 1 in thumb mode, so add 1 to all symbols
    ASM("\t\n\
        .word   __boot_stack__  + 1     // Stack pointer at reset               \t\n\
        .word   _mcu_start + 1          // Reset                                \t\n\
        .word   _int_entry + 1          // NMI                                  \t\n\
        .word   _int_entry + 1          // Hard fault                           \t\n\
        .word   _int_entry + 1          // Memory management fault              \t\n\
        .word   _int_entry + 1          // Bus fault                            \t\n\
        .word   _int_entry + 1          // Usage fault                          \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _svc_handler + 1        // SVCall                               \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _int_entry + 1          // Reserved                             \t\n\
        .word   _int_entry + 1          // PendSV                               \t\n\
        .word   _int_entry + 1          // Systick                              \t\n\
        .word   _int_entry + 1          // IRQ0                                 \t\n\
        .word   _int_entry + 1          // IRQ1                                 \t\n\
        .word   _int_entry + 1          // IRQ2                                 \t\n\
        .word   _int_entry + 1          // IRQ3                                 \t\n\
        .word   _int_entry + 1          // IRQ4                                 \t\n\
        .word   _int_entry + 1          // IRQ5                                 \t\n\
        .word   _int_entry + 1          // IRQ6                                 \t\n\
        .word   _int_entry + 1          // IRQ7                                 \t\n\
        .word   _int_entry + 1          // IRQ8                                 \t\n\
        .word   _int_entry + 1          // IRQ9                                 \t\n\
        .word   _int_entry + 1          // IRQ10                                \t\n\
        .word   _int_entry + 1          // IRQ11                                \t\n\
        .word   _int_entry + 1          // IRQ12                                \t\n\
        .word   _int_entry + 1          // IRQ13                                \t\n\
        .word   _int_entry + 1          // IRQ14                                \t\n\
        .word   _int_entry + 1          // IRQ15                                \t\n\
        .word   _int_entry + 1          // IRQ16                                \t\n\
        .word   _int_entry + 1          // IRQ17                                \t\n\
        .word   _int_entry + 1          // IRQ18                                \t\n\
        .word   _int_entry + 1          // IRQ19                                \t\n\
        .word   _int_entry + 1          // IRQ20                                \t\n\
        .word   _int_entry + 1          // IRQ21                                \t\n\
        .word   _int_entry + 1          // IRQ22                                \t\n\
        .word   _int_entry + 1          // IRQ23                                \t\n\
        .word   _int_entry + 1          // IRQ24                                \t\n\
        .word   _int_entry + 1          // IRQ25                                \t\n\
        .word   _int_entry + 1          // IRQ26                                \t\n\
        .word   _int_entry + 1          // IRQ27                                \t\n\
        .word   _int_entry + 1          // IRQ28                                \t\n\
        .word   _int_entry + 1          // IRQ29                                \t\n\
        .word   _int_entry + 1          // IRQ30                                \t\n\
        .word   _int_entry + 1          // IRQ31                                \t\n\
        .word   _int_entry + 1          // IRQ32                                \t\n\
        .word   _int_entry + 1          // IRQ33                                \t\n\
        .word   _int_entry + 1          // IRQ34                                \t\n\
        .word   _int_entry + 1          // IRQ35                                \t\n\
        .word   _int_entry + 1          // IRQ36                                \t\n\
        .word   _int_entry + 1          // IRQ37                                \t\n\
        .word   _int_entry + 1          // IRQ38                                \t\n\
        .word   _int_entry + 1          // IRQ39                                \t\n\
        .word   _int_entry + 1          // IRQ40                                \t\n\
        .word   _int_entry + 1          // IRQ41                                \t\n\
        .word   _int_entry + 1          // IRQ42                                \t\n\
        .word   _int_entry + 1          // IRQ43                                \t\n\
        .word   _int_entry + 1          // IRQ44                                \t\n\
        .word   _int_entry + 1          // IRQ45                                \t\n\
        ");
}
