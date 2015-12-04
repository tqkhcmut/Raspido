<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html>
	<head>
		<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
		<title>Sensor values table</title>
		<link rel="stylesheet" type="text/css" href="style.css" />
	</head>
	
	<body>
		<h1>Sensor data</h1>
		<div id="wrapper">
			<table>
				<tr>
					<th>Record Count</th>
					<th>Record Time</th>
					<th>Unique Number</th>
					<th>Gas</th>
					<th>Lighting</th>
					<th>Temperature</th>
				</tr>
				<?php
					class MyDB extends SQLite3
					{
						function __construct()
						{
							$this->open('thesis.db');
						}
					}
					$db = new MyDB();
					if(!$db)
					{
								$res = 
								"<tr>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								</tr>";
								echo $res;
					} 
					else 
					{
						$db->busyTimeout(500);
						$sql =<<<EOF
						SELECT * FROM sensor_values order by record_count desc limit 100;
EOF;
						$ret = $db->query($sql);
						if ($ret)
						{
							while($row = $ret->fetchArray(SQLITE3_ASSOC) )
							{
								$res = 
								"<tr>
								<td>" . $row['record_count'] . "</td>
								<td>" . $row['record_time'] . "</td>
								<td>" . $row['unique_number'] . "</td>
								<td>" . $row['gas'] . " ppm</td>
								<td>" . $row['lighting'] . " lux</td>
								<td>" . $row['tempc'] . " &deg;C</td>
								</tr>";
								echo $res;
							}
						}
						else
						{
								$res = 
								"<tr>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								<td>NULL</td>
								</tr>";
								echo $res;
						}
					}
					
					$db->close();
				?>
			</table>
		</div>
	</body>
</html>
