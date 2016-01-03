<?php
	$unique_num = $_POST["sensor"];
	
	class MyDB extends SQLite3
	{
		function __construct()
		{
			$this->open('thesis.db', SQLITE3_OPEN_READONLY);
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
		SELECT * FROM sensor_values where unique_number == $unique_num ORDER BY record_count DESC LIMIT 100;
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