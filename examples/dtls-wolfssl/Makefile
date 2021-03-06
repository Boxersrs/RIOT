# name of your application
APPLICATION = dtls_wolfssl

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# wolfSSL supports 32-bit architectures only
BOARD_BLACKLIST := arduino-duemilanove arduino-mega2560 arduino-uno chronos \
                   jiminy-mega256rfr2 mega-xplained msb-430 msb-430h telosb \
                   waspmote-pro wsn430-v1_3b wsn430-v1_4 z1

BOARD_INSUFFICIENT_MEMORY := airfy-beacon b-l072z-lrwan1 bluepill calliope-mini \
                             cc2650-launchpad cc2650stk hifive1 maple-mini \
                             microbit nrf51dongle nrf6310 nucleo-f031k6 \
                             nucleo-f042k6 nucleo-f303k8 nucleo-l031k6 nucleo-f030r8 \
                             nucleo-f070rb nucleo-f072rb nucleo-f103rb nucleo-f302r8 nucleo-f334r8 \
                             nucleo-l053r8 nucleo-l073rz opencm904 \
                             spark-core stm32f0discovery stm32mindev yunjia-nrf51822

# Include packages that pull up and auto-init the link layer.
# NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
# Specify the mandatory networking modules for IPv6 and UDP
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_sock_udp

# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands

USEPKG += wolfssl
USEMODULE += wolfcrypt
USEMODULE += wolfssl
USEMODULE += wolfcrypt-test
USEMODULE += wolfcrypt_ecc
USEMODULE += wolfssl_dtls

#CFLAGS+=-g -ggdb3

CFLAGS += -DDTLS_DEFAULT_PORT=$(DTLS_PORT) -DDTLS_WOLFSSL

# FIXME: This is a temporary patch taken from the TinyDTLS example to ensure proper stack allocation
CFLAGS += -DTHREAD_STACKSIZE_MAIN=\(3*THREAD_STACKSIZE_DEFAULT\)

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
CFLAGS += -DDEVELHELP

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 0

include $(RIOTBASE)/Makefile.include
