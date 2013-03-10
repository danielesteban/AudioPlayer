#!/usr/local/bin/node

var fs = require('fs'),
	mp3s = fs.readdirSync('./mp3')
	sampleRate = 44100,
	sampleBits = 8,
	soxEffects = ['vol', '0.9'],
	wav2cBinary = '../../Downloads/wav2c/wav2c';

function genAudio() {
	if(!mp3s.length) return console.log("Done!");
	f = mp3s.shift();
	if(f.substr(f.lastIndexOf('.') + 1) == 'mp3') {
		var fn = f.substr(0, f.length - 4),
			mpg123 = require('child_process').spawn('mpg123', [
				'-w./wav/' + fn + '.wav',
				'./mp3/' + f
			]);

		mpg123.on('exit', function (code) {
			if(code !== 0) return;
			var sox = require('child_process').spawn('sox', [
					'./wav/' + fn + '.wav',
					'-c1',
					'-r' + sampleRate,
					'-eunsigned-integer',
					'-b' + sampleBits,
					'./wav/' + sampleRate + '-' + fn + '.wav'
				].concat(soxEffects));

			sox.on('exit', function (code) {
				if(code !== 0) return;
				var wav2c = require('child_process').spawn(wav2cBinary, [
					'./wav/' + sampleRate + '-' + fn + '.wav',
					'./wav/' + fn + '.h',
					fn
				]);

				wav2c.on('exit', function (code) {
					fs.unlinkSync('./wav/' + sampleRate + '-' + fn + '.wav');
					fs.unlinkSync('./wav/' + fn + '.wav');
					if(code !== 0) return;

					var w = fs.readFileSync('./wav/' + fn + '.h', 'utf8');
					w = w.substr(w.indexOf("={") + 2);
					w = w.substr(0, w.indexOf(", }"));
					w = eval("new Buffer([" + w + "])");
					fs.writeFileSync('./raw/' + fn + '.raw', w);
					fs.unlinkSync('./wav/' + fn + '.h');
					console.log(fn);
					genAudio();
				});

				wav2c.stderr.on('data', function (data) {
					console.log('wav2c stderr: ' + data);
				});
			});

			sox.stderr.on('data', function (data) {
				console.log('sox stderr: ' + data);
			});
		});

		mpg123.stderr.on('data', function (data) {
			console.log('mpg123 stderr: ' + data);
		});
	} else genAudio();
}

genAudio();
