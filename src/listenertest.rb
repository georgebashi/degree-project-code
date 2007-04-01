#!/usr/bin/env ruby
require 'find'
# mplayer -ss 00:01:00 -endpos 00:00:15

songs = Array.new

Find.find("../test/") { |file|
	if file =~ /\.vec$/
		songs << file
	end
}

puts "Loaded #{songs.size} songs"
puts "\n"

def save(key, similar, not_similar)
	output = File.new("results.out", "a")
	output.puts "#{key} * #{similar} * #{not_similar}"
	output.close
end

while true
	puts
	puts "-------------------------------------------------------------"
	puts "\a"
	key = songs[rand(songs.size)].chop.chop.chop.chop
	a = songs[rand(songs.size)].chop.chop.chop.chop
	b = songs[rand(songs.size)].chop.chop.chop.chop

	puts "Listen to this song:"
	`mplayer -ss 00:01:00 -endpos 00:00:15 "#{key}"`
	puts "Now choose which of the following two songs is most similar to the first:"
	done = false
	played = false
	response = ""
	until done
		print "Song A"
		$stdout.flush
		unless played
			sleep 0.2
			`beep -f 300 -r 1 -d 100 -l 400`
			sleep 0.2
			`mplayer -ss 00:01:00 -endpos 00:00:5 "#{a}"`
		end
		print " or Song B"
		$stdout.flush
		unless played
			sleep 0.2
			`beep -f 300 -r 2 -d 100 -l 400`
			sleep 0.2
			`mplayer -ss 00:01:00 -endpos 00:00:5 "#{b}"`
		end
		puts "? (enter L followed by K, A or B to hear the excerpts again, S to skip or Q to quit)"
		played = true
		
		response = gets.chomp.upcase
		case response
			when "A"
				save key, a, b
				done = true
			when "B"
				save key, b, a
				done = true
			when "LK", "L K"
				`mplayer -ss 00:01:00 -endpos 00:00:3 "#{key}" 1>&2`
			when "LA", "L A"
				`mplayer -ss 00:01:00 -endpos 00:00:3 "#{a}" 1>&2`
			when "LB", "L B"
				`mplayer -ss 00:01:00 -endpos 00:00:3 "#{b}" 1>&2`
			when "S"
				done = true
				next
			when "Q"
				break
		end
	end
	break if response == "Q"
end

puts "Thanks!"
