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

class EchoProfile < Bluez::Profile

  def connection(path, fd)
    @path = path
    @fd = fd

    sock = Socket.for_fd(fd)

    puts "new echo connection #{path}"
    sock.write("HELLO\n\n")
    loop do
      data = sock.readline  rescue break
      sock.write("echo: #{data}") rescue break
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
  role: Bluez::Profile::Server,
  channel: 22
  })

puts 'running echo profile ...'

profile.run

puts "exiting.."
