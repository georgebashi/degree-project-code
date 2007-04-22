#!/usr/bin/ruby

# $Id$

require 'find'
require 'rubygems'
require 'inline'

class Fixnum
	N_BYTES = [42].pack('i').size
	N_BITS = N_BYTES * 8
	MAX = 2 ** (N_BITS - 2) - 1
	MIN = -MAX - 1
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
	
	def max
		cur = 0
		self.each { |num|
			cur = num if num > cur
		}
		cur
	end
	
	def min
		cur = Fixnum::MAX
		self.each { |num|
			cur = num if num < cur
		}
		cur
	end
end

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

@results = Array.new

File.open("results.txt").each { |line|
	entry = Array.new
	entry += line.split(" * ")
	@results << entry
}

@song_count = (`./generator -v --dir=../test --similar=1 ../test/Hot\ Chip/Coming\ on\ Strong/Coming\ on\ Strong-11-One\ One\ One.ogg.vec`.split)[0].to_i
puts "Generator has #{@song_count} songs."

for comparison in ["ordered", "ordered-area", "sorted", "total"]
	pos_similar = Array.new
	pos_not_similar = Array.new
	@results.each { |result|
		playlist = get_similar(result[0], comparison)
		pos_similar << playlist_position(playlist, result[1])
		pos_not_similar << playlist_position(playlist, result[2])
	}

	similar_avg = pos_similar.average
	similar_stdev = pos_similar.stdev(similar_avg)
	similar_high = pos_similar.max
	similar_low = pos_similar.min
    not_similar_avg = pos_not_similar.average
    not_similar_stdev = pos_not_similar.stdev(not_similar_avg)
    not_similar_high = pos_not_similar.max
    not_similar_low = pos_not_similar.min

	score = similar_avg + (@song_count - not_similar_avg)
	puts "#{comparison} has similar_avg #{similar_avg} stdev #{similar_stdev} high #{similar_high} low #{similar_low}, not_similar_avg #{not_similar_avg} stdev #{not_similar_stdev} high #{not_similar_high} low #{not_similar_low}, score #{score}"
end
