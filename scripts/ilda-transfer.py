from cobs import cobs
import click
import serial

@click.command()
@click.argument('port', help='Serial port connected to STM32.', type=click.File('rb'))
@click.argument('file', help='ILDA file to transfer.', type=str)
def ilda_transfer(port, file):

    # Read file and encode to COBS
    ilda_data = file.read()
    cobs_ilda_data = cobs.encode(bytearray(ilda_data)) + b'\x00'

    print('Encoded lenght: {}\n'.format(len(cobs_ilda_data)))
    
    s = serial.Serial(port, 9600, timeout=5) # Timeout 1 second for reads

    s.write(cobs.encode(bytearray('RTS')) + b'\x00') # write request to send to STM
    
    ready = cobs.decode(s.read_until(expected='\x00'))

    if ready == b'CTS':
        print('Transfering ILDA file...\n')
        s.write(cobs_ilda_data)
        print('Transfer complete!\n')
    

if __name__ == '__main__':
    ilda_transfer()
