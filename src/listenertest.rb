#!/usr/bin/env ruby

# $Id$

# Music Quiz listener test
# Plays extracts of random songs from the media library and asks the user which are most similar

require 'find'

songs = Array.new

Find.find("../test/") { |file|
	if file =~ /\.vec$/
		songs << file
	end
}

puts "Loaded #{songs.size} songs\n"

# play an audio file with a number of beeps
def play(file, num_beeps = 0)
	num_beeps.times do
		`beep -f 300 -r 1 -d 100 -l 400 > /dev/null 2>&1`
		sleep 0.2
	end
	`mplayer -af volnorm -ss 00:01:00 -endpos 00:00:5 "#{file}" > /dev/null 2>&1`
	`mplayer -af volnorm -ss 00:02:00 -endpos 00:00:5 "#{file}" > /dev/null 2>&1`
	sleep 0.2
end

# get stored info on a song
def info(file)
	`mplayer -identify -endpos 0:00:00 "#{file}" | grep ID_CLIP_INFO_VALUE | cut -c 21- | xargs echo 1>&2`
end

# save a result
def save(similar_a, similar_b, not_similar)
	output = File.new("results.txt", "a")
	output.puts "#{similar_a} * #{similar_b} * #{not_similar}"
	output.close
end

# run the test
while true
	puts "\n-------------------------------------------------------------\n"
	a = songs[rand(songs.size)].chop.chop.chop.chop
	b = songs[rand(songs.size)].chop.chop.chop.chop
	c = songs[rand(songs.size)].chop.chop.chop.chop

	puts "Listen to these three extracts, and choose which two are the most similar:"
	
	done = false
	played = false
	response = ""
	
	until done
		print "Song A"; $stdout.flush
		play a, 1 unless played
		
		print ", Song B"; $stdout.flush
		play b, 2 unless played
		
		print " or Song C"; $stdout.flush
		play c, 3 unless played

		puts "? (enter ? for help)"
		played = true
		
		response = gets.chomp.upcase
		case response
			when "?"
				puts "[A,B,C][A,B,C] - Answer"
				puts "L[A,B,C] - Listen to clip again"
				puts "I[A,B,C] - Get track info"
				puts "S - Skip"
				puts "Q - Quit"
			when "AB", "BA"
				save a, b, c
				done = true
			when "BC", "CB"
				save b, c, a
				done = true
			when "AC", "CA"
				save a, c, b
				done = true
			when "LA"
				play a
			when "LB"
				play b
			when "LC"
				play c
			when "IA"
				info a
			when "IB"
				info b
			when "IC"
				info c
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
