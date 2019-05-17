#!/usr/bin/env ruby
require 'pty'

subcmd = ARGV.shift
script = STDIN.readlines.map {|l| l.chomp }
STDOUT.sync = true

begin
  PTY.spawn(subcmd) do |pin, pout, pid|
    pin.sync = true
    pout.sync = true

    Thread.new do
      sleep(1)
      script.each do |cmd|
        case cmd
        when /!sleep (\d+)/
          sleep($1.to_i)
        when /!break/
          pout.puts('')
        when /!kill (\w+)/
          sig = $1
          case sig
          when 'INT';  STDOUT.print('^C')
          when 'QUIT'; STDOUT.print('^]')
          when 'TSTP'; STDOUT.print('^Z')
          end
          Process.kill(sig, pid)
        when /!eof/
          STDOUT.print('^D')
          #pout.print 0x04.chr
          pout.close
        else
          pout.puts cmd
        end
      end

      sleep(1)
      Process.kill("KILL", pid) rescue nil
    end

    loop do
      print pin.sysread(512)
    end
  end
rescue EOFError
end
