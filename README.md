# register profiles with bluez

this gem allows easy creation of bluetooth services.
For example create a serial link over to another device using bluetooth.


Clive Andrews 2021

## dependencies

* Linux/Unix type system
* bluez 5
* glib-2


## API

example code:

    class MyProfile < Bluez::Profile

      # This method gets called when the service daemon
      # unregisters the profile. A profile can use it to do
      # cleanup tasks.

      def release

      end

      # This method gets called when a new service level
      # connection has been made and authorized.

      def  connection(device, fd, fd_properties)

      end

      # This method gets called when a profile gets
      # disconnected.
      #
      # The file descriptor is no longer owned by the service
      # daemon and the profile implementation needs to take
      # care of cleaning up all connections.
      #
      # If multiple file descriptors are indicated via
      # NewConnection, it is expected that all of them
      # are disconnected before returning from this
      # method call.

      def disconnection(device)

      end

    end

    path = '/my/dbus/path'
    uuid = '1101'

    options = {
      name: 'my profile',
      channel: 3
      connect: false
    }

    profile = MyProfile.new(path, uuid, options)
    profile.run   # enters loop and blocks here.

    profile.stop  # stops the loop


path: string

    the dbus object path of the profile.
    eg: "/serial/special/profile"

uuid: string

    the dbus profile uuid


Available options:

        name: string

          Human readable name for the profile

        service: string

          The primary service class UUID
          (if different from the actual
           profile UUID)

        role: string

          For asymmetric profiles that do not
          have UUIDs available to uniquely
          identify each side this
          parameter allows specifying the
          precise local role.

          Possible values:

              Bluez::Profile::Client
              Bluez::Profile::Server

        channel: int

          RFCOMM channel number that is used
          for client and server UUIDs.

          If applicable it will be used in the
          SDP record as well.

        psm: int

          PSM number that is used for client
          and server UUIDs.

          If applicable it will be used in the
          SDP record as well.

        authentication: boolean

          Pairing is required before connections
          will be established. No devices will
          be connected if not paired.

        authorization: boolean

          Request authorization before any
          connection will be established.

        connect: boolean

          In case of a client UUID this will
          force connection of the RFCOMM or
          L2CAP channels when a remote device
          is connected.

        record: string

          Provide a manual SDP record.

        version: int

          Profile version (for SDP record)

        features: int

          Profile features (for SDP record)

## examples

See the `/examples` folder.
