#!/usr/bin/ruby

# $Id$

require 'find'
require 'rubygems'
require 'inline'

@results = Array.new

@song_count = (`./generator -v --dir=../test --similar=1 ../test/Hot\ Chip/Coming\ on\ Strong/Coming\ on\ Strong-11-One\ One\ One.ogg.vec`.split)[0].to_i
puts "Generator has #{@song_count} songs."

File.open("results.txt").each { |line|
	entry = Array.new
	entry += line.split(" * ")
	@results << entry
}

def get_similar(track, comparison)
	playlist = Array.new
	`./generator --dir=../test --comparison=#{comparison} --similar=#{@song_count} "#{track}"`.each { |line|
		playlist << line
	}
	playlist
end

def playlist_position(playlist, track)
	pos = 0
	for i in 0...playlist.size
		break if playlist.at(i).strip == track.strip
		pos += 1
	end
	pos
end

def rate(key, similar, not_similar, comparison)
	playlist = get_similar(key, comparison)
	playlist_position(playlist, similar) + (@song_count - playlist_position(playlist, not_similar))
end

def test(comparison)
	scores = Array.new
	@results.each { |result|
		scores << rate(result[0], result[1], result[2], comparison)
	}
	scores
end

class Array
	def average
		total = 0
		self.each { |num|
			total += num
		}
		Float(total) / self.size
	end
	
	def stdev(avg)
		total = 0
		self.each { |num|
			total += (num - avg) ** 2
		}
		Math.sqrt((1.0 / self.size) * total)
	end
end

for comparison in ["ordered", "ordered-area", "sorted", "total"]
	scores = test(comparison)
	score_avg = scores.average
	score_stdev = scores.stdev(score_avg)
	puts "#{comparison} has avg #{score_avg} and stdev #{score_stdev}"
end
