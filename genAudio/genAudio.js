#!/usr/local/bin/node

var fs = require('fs'),
	mp3s = fs.readdirSync('./mp3')
	sampleRate = 32000,
	sampleBits = 16,
	soxEffects = [],
	fileCount = (fs.existsSync('./raw') ? fs.readdirSync('./raw/').length : 0) + 1;

function genAudio() {
	if(!mp3s.length) return console.log("Done!");
	f = mp3s.shift();
	if(f.substr(f.lastIndexOf('.') + 1) == 'mp3') {
		!fs.existsSync('./wav') && fs.mkdirSync('./wav', '0755');
		var fn = fileCount + '';
		while(fn.length < 4) fn = '0' + fn; 
		var //fn = f.substr(0, f.length - 4),
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
					'-esigned-integer',
					'-b' + sampleBits,
					'./wav/' + sampleRate + '-' + fn + '.wav'
				].concat(soxEffects));

			sox.on('exit', function (code) {
				if(code !== 0) return;
				!fs.existsSync('./raw') && fs.mkdirSync('./raw', '0755');
				var wav2c = require('child_process').spawn('./genAudio', [
					'./wav/' + sampleRate + '-' + fn + '.wav',
					'./raw/' + fn + '.raw',
					fn
				]);

				wav2c.on('exit', function (code) {
					fs.unlinkSync('./wav/' + sampleRate + '-' + fn + '.wav');
					fs.unlinkSync('./wav/' + fn + '.wav');
					if(code !== 0) return;
					console.log(f);
					fileCount++;
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
			//Commented out.. too verbose.
			//console.log('mpg123 stderr: ' + data);
		});
	} else genAudio();
}

genAudio();
