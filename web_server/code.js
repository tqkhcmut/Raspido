var gas_timeline = new TimeSeries();
var lighting_timeline = new TimeSeries();
var tempc_timeline = new TimeSeries();
var windowGap = 40;

$(function() {
	$("#status").text("");
    document.getElementById('gas').width  = window.innerWidth-windowGap;
    document.getElementById('lighting').width  = window.innerWidth-windowGap;
    document.getElementById('tempc').width  = window.innerWidth-windowGap;
	get_sensors_value();
	setInterval("get_sensors_value()", 1000);
	
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


function get_sensors_value()
{
   $.ajax({
		url: "sensors.php",
		type: "post",
		data: { counter:"0" }
	}).done(function(values)
	{      	
      	gas_timeline.append(new Date().getTime(), values[0]);
		$("#gas_value").text("" + values[0]);
      	lighting_timeline.append(new Date().getTime(), values[1]);
		$("#light_value").text("" + values[1]);
      	tempc_timeline.append(new Date().getTime(), values[2]);
		$("#tempc_value").text("" + values[2]);
	});
}

