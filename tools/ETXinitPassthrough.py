#!/usr/bin/env python
import serial, time, sys, glob
import argparse
import streamexpect


def dbg_print(line=''):
    sys.stdout.write(line + '\n')
    sys.stdout.flush()

def serial_ports():
    """ Lists serial port names

        :raises Exception:
            On unsupported or unknown platforms
        :returns:
            A list of the serial ports available on the system
    """
    result = []
    ports = []

    try:
        from serial.tools.list_ports import comports
        if comports:
            print("  ** Searching for radios **")
            __ports = list(comports())
            for port in __ports:
                if (port.vid and port.vid == 0x0483):
                    print("      > Radio found from '%s'" % port.device)
                    ports.append(port.device)
    except ImportError:
        pass

    if not ports:
        print("  ** No Radio found, Please specify port **")

    for port in ports:
        try:
            s = serial.Serial(port)
            s.close()
            result.append(port)
        except (OSError, serial.SerialException) as error:
            if "permission denied" in str(error).lower():
                raise Exception("You don't have permission to use serial port!")
            pass
    result.reverse()
    return result

def get_serial_port(debug=True):
    result = serial_ports()
    if debug:
        print()
        print("Detected the following serial ports on this system:")
        for port in result:
            print("  %s" % port)
        print()

    if len(result) == 0:
        raise Exception('No valid serial port detected or port already open')

    return result[0]


def etx_passthrough_init(port, requestedBaudrate):
    sys.stdout.flush()
    dbg_print("======== PASSTHROUGH INIT ========")
    dbg_print("  Trying to initialize %s @ %s" % (port, requestedBaudrate))

    s = serial.Serial(port=port, baudrate=requestedBaudrate,
        bytesize=8, parity='N', stopbits=1,
        timeout=1, xonxoff=0, rtscts=0)

    with streamexpect.wrap(s) as rl:
        rl.flush()
        rl.write(b"set pulses 0\n")
        rl.expect_bytes(b"set: ", timeout=1.0)
        rl.expect_bytes(b"> ", timeout=1.0)
        rl.write(b"set rfmod 0 power off\n")
        rl.expect_bytes(b"set: ", timeout=1.0)
        rl.expect_bytes(b"> ", timeout=1.0)
        time.sleep(.5)
        if not args.backpack:
            rl.write(b"set rfmod 0 bootpin 1\n")
            rl.expect_bytes(b"set: ", timeout=1.0)
            rl.expect_bytes(b"> ", timeout=1.0)
            time.sleep(.1)
        rl.write(b"set rfmod 0 power on\n")
        rl.expect_bytes(b"set: ", timeout=1.0)
        rl.expect_bytes(b"> ", timeout=1.0)
        time.sleep(1)
        rl.write(b"set rfmod 0 bootpin 1\n")
        rl.expect_bytes(b"set: ", timeout=1.0)
        rl.expect_bytes(b"> ", timeout=1.0)
        time.sleep(1)
        rl.write(b"set rfmod 0 bootpin 0\n")
        rl.expect_bytes(b"set: ", timeout=1.0)
        rl.expect_bytes(b"> ", timeout=1.0)

        cmd = "serialpassthrough rfmod 0 %s" % requestedBaudrate

        dbg_print("Enabling serial passthrough...")
        dbg_print("  CMD: '%s'" % cmd)
        rl.write(cmd.encode("utf-8"))
        rl.write(b'\n')
        time.sleep(.2)

    s.close()
    if args.backpack:
        dbg_print("======== PASSTHROUGH TO BACKPACK READY ========")
    else:
        dbg_print("======== PASSTHROUGH READY ========")

def init_passthrough(source, target, env):
    env.AutodetectUploadPort([env])
    port = env['UPLOAD_PORT']
    etx_passthrough_init(port, env['UPLOAD_SPEED'])

if __name__ == '__main__':
    parser = argparse.ArgumentParser(
        description="Initialize EdgeTX passthrough to internal module")
    parser.add_argument("-b", "--baud", type=int, default=115200,
        help="Baud rate for passthrough communication")
    parser.add_argument("-p", "--port", type=str,
        help="Override serial port autodetection and use PORT")
    parser.add_argument("--backpack", action="store_true",
        help="backpack bootloader passthrough")
    args = parser.parse_args()

    if (args.port == None):
        args.port = get_serial_port()

    etx_passthrough_init(args.port, args.baud)
