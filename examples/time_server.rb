# frozen_string_literal: true

# this will forward data from a usb time device over a
# bluetooth connection.
#
# C. Andrews 2021

require 'bluez/profile'
require 'socket'

MY_UUID="c7a4f535-a70a-4817-94de-2e43fb0badc2"

# define our serial profile class here which will
# just read from our usb device and forward the raw info
# over the bluetooth serial profile channel.

class TimeProfile < Bluez::Profile

  def connection(path, fd)
    @path = path
    @fd = fd

    sock = Socket.for_fd(fd)

    puts "new time connection #{path}"
    loop do
      data = "#{Time.now.to_s}\n"
      sock.write(data) rescue break
      sleep(1)
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

profile = TimeProfile.new('/serial/bluetooth/timeprofile', '1101', {
  name: 'Time Data Serial Port',
  service: MY_UUID,
  channel: 3,
  role: Bluez::Profile::Server,
  })

puts 'running time profile ...'

profile.run

puts "exiting.."
