# frozen_string_literal: true

# this will echo data back to client.
#
#
# C. Andrews 2021

require 'bluez/profile'
require 'socket'


# define our serial profile class here which will
# just read from our usb device and forward the raw info
# over the bluetooth serial profile channel.
Thread.abort_on_exception = true

class EchoProfile < Bluez::Profile

  def connection(path, fd)
    @path = path
    @fd = fd
    @run = true

    sock = Socket.for_fd(fd)

    puts "new echo connection"
    Thread.new(sock){ |sock|
      loop do
        data = sock.readline rescue break
        print( "==>" + data )
      end
      @run=false
    }

    while(@run) do
      mymessage = gets
      sock.write(mymessage) rescue break
    end
    puts "connection broken"
  end

  def release
  end

  def disconnection
  end

end

# now create the profile under a unique object dbus path. The
# bluetooth uuid for the serial profile is '1101'

profile = EchoProfile.new('/serial/bluetooth/echoprofile', '1101', {
  name: 'Echo Serial Port',
  role: Bluez::Profile::Client,
  channel: 22,
  connect: true
  })

puts 'running echo profile ...'

profile.run

puts "exiting.."
