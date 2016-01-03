var gas_timeline = new TimeSeries();
var lighting_timeline = new TimeSeries();
var tempc_timeline = new TimeSeries();
var __unique_number = 1;
var get_current_values_interval;
var get_table_values_interval;

var sound;

// sound status:
// 0 stop;
// 1 unmute;
// 2 mute;
var sound_status = 0;
var gas_limit = 1500.0, light_limit = 100.0, tempc_limit = 40.0;

function get_limit()
{
	$.ajax({
		url: "limit.php",
		type: "post",
		data: { sensor:__unique_number }
	}).done(function(limit_values)
	{
		gas_limit = limit_values[0];
		light_limit = limit_values[1];
		tempc_limit = limit_values[2];
	});
}

function get_active_host()
{
	$.ajax({
		url: "active_host.php",
		type: "post",
		data: {}
	}).done(function(active_host)
	{
		var tmp_ul_html;
		var update_unique = 1;
		$("#primary").html("");
		for (var n=0; n<active_host.length; n++)
		{
			if (__unique_number == active_host[n])
				update_unique = 0;
			tmp_ul_html =
				'<ul class="host">\
                    <li class="name">Host ' + active_host[n] + '</li>\
                    <li class="con"><input type="image" src="images/monitoring5.png" alt="images not found" onClick="update_view(' + active_host[n] + ', \'monitor\')"/></li>\
                    <li class="con"><input type="image" src="images/data_table5.png" alt="images not found" onClick="update_view(' + active_host[n] + ', \'table\')"/></li>\
                </ul>';
			$("#primary").append(tmp_ul_html);
		}
		if (active_host.length > 0 && update_unique == 1)
		{
			__unique_number = active_host[0];
			update_view(__unique_number, 'monitor');
		}
	});
}
function stopSound()
{
	var res = confirm("Warning off?");
	if (res == true)
	{
		if (sound_status == 1)
		{
			sound.mute();
			sound_status = 2;
			$("#gas_warning").hide();
			$("#lighting_warning").hide();
			$("#tempc_warning").hide();
		}
	}
}

$(window).resize(function(e) {
    document.getElementById('gas').width  = $(window).width() * 0.55 * 0.85 * 0.95;
    document.getElementById('lighting').width  = $(window).width() * 0.55 * 0.85 * 0.95;
    document.getElementById('tempc').width  = $(window).width() * 0.55 * 0.85 * 0.95;
});

$(function() {
	sound = new Howl({
		urls: ['siren.mp3'], loop: true, autoplay: true
	});
	$("#show_table").hide();
	$("#show_monitor").show();
	sound.play();
	sound.mute();
	sound_status = 2;
	$("#gas_warning").hide();
	$("#lighting_warning").hide();
	$("#tempc_warning").hide();
	//__unique_number = $("#sensor_title").text().replace(/^\D+|\D+$/g, "");
	$("#status").text("");
    document.getElementById('gas').width  = $(window).width() * 0.55 * 0.85 * 0.95;
    document.getElementById('lighting').width  = $(window).width() * 0.55 * 0.85 * 0.95;
    document.getElementById('tempc').width  = $(window).width() * 0.55 * 0.85 * 0.95;
	
	get_active_host();
	get_limit();
	setInterval("get_active_host()", 2000);
	setInterval("get_limit()", 2000);
	
	get_sensors_value();
	get_current_values_interval = setInterval("get_sensors_value()", 1000);
	get_sensors_table();
	get_table_values_interval = setInterval("get_sensors_table()", 1000);
	
	var gasline = new SmoothieChart(
  	{
		millisPerPixel: 80,
     		grid:
     		{
        		strokeStyle: '#555555',
        		verticalSections: 10 
     		}
  	});
  	gasline.addTimeSeries(gas_timeline,
    {
        strokeStyle: 'rgba(255, 0, 0, 1)',
        fillStyle: 'rgba(255, 0, 0, 0.2)',
        lineWidth: 3
    });
  	gasline.streamTo(document.getElementById('gas'), 1000);
  	
	var lighttime = new SmoothieChart(
  	{
		millisPerPixel: 80,
     		grid:
     		{
        		strokeStyle: '#555555',
        		verticalSections: 10 
     		}
  	});
  	lighttime.addTimeSeries(lighting_timeline,
    {
        strokeStyle: 'rgba(0, 255, 0, 1)',
        fillStyle: 'rgba(0, 255, 0, 0.2)',
        lineWidth: 3
    });
  	lighttime.streamTo(document.getElementById('lighting'), 1000);
  	
	var tempctime = new SmoothieChart(
  	{
		millisPerPixel: 80,
     		grid:
     		{
        		strokeStyle: '#555555',
        		verticalSections: 10 
     		}
  	});
  	tempctime.addTimeSeries(tempc_timeline,
    {
        strokeStyle: 'rgba(0, 0, 255, 1)',
        fillStyle: 'rgba(0, 0, 255, 0.2)',
        lineWidth: 3
    });
  	tempctime.streamTo(document.getElementById('tempc'), 1000);
});

function get_sensors_table()
{
	$.ajax({
		url: "sensors.php",
		type: "post",
		data: { sensor:__unique_number }
	}).done(function(sensors_tb_row)
	{
		$("#table").html(' \
				<tr> \
					<th>Record Count</th> \
					<th>Record Time</th> \
					<th>Unique Number</th> \
					<th>Gas</th> \
					<th>Lighting</th> \
					<th>Temperature</th> \
				</tr>' + sensors_tb_row);
	});
}
function update_view(unique, type)
{
	clearInterval(get_current_values_interval);
	clearInterval(get_table_values_interval);
	__unique_number = unique;
	$("#show_table").hide();
	$("#show_monitor").hide();
	if (type == "monitor")
	{
		$("#show_monitor").show();
		$("#show_monitor_title").html('<p class="tittle_monitoring"><strong>Host ' + __unique_number + '</strong> : Monitoring</p>')
	}
	else 
	{
		$("#show_table").show();
		$("#show_table_title").html('<p class="tittle_table"><strong>Host ' + __unique_number + '</strong> : Data Table</p>')
	}
	get_current_values_interval = setInterval("get_sensors_value()", 1000);
	get_table_values_interval = setInterval("get_sensors_table()", 1000);
}

function get_sensors_value()
{
   $.ajax({
		url: "request.php",
		type: "post",
		data: { sensor:__unique_number }
	}).done(function(values)
	{      	
      	gas_timeline.append(new Date().getTime(), values[0]);
		$("#gas_value").text("" + values[0]);
      	lighting_timeline.append(new Date().getTime(), values[1]);
		$("#light_value").text("" + values[1]);
      	tempc_timeline.append(new Date().getTime(), values[2]);
		$("#tempc_value").text("" + values[2]);
		
		$("#gas_warning").hide();
		$("#lighting_warning").hide();
		$("#tempc_warning").hide();
		if (values[0] >= gas_limit)
		{
			if (sound_status == 2)
			{
				sound.unmute();
				sound_status = 1;
			}
			$("#gas_warning").show();
		}
		if (values[1] >= light_limit)
		{
			if (sound_status == 2)
			{
				sound.unmute();
				sound_status = 1;
			}
			$("#lighting_warning").show();
		}
		if (values[2] >= tempc_limit)
		{
			if (sound_status == 2)
			{
				sound.unmute();
				sound_status = 1;
			}
			$("#gas_warning").show();
		}
		if (values[0] < gas_limit && values[1] < light_limit && values[2] < tempc_limit)
		{
			//if (sound_status == 1)
			//{
				sound.mute();
				sound_status = 2;
			//}
		}
	});
}

