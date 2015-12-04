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
		$res = [0, 0, 0];
		header('Content-Type: application/json');
		echo json_encode($res);
	} 
	else 
	{
		$db->busyTimeout(500);
		$sql =<<<EOF
		SELECT * FROM sensor_values order by record_count desc limit 1;
EOF;
		$ret = $db->query($sql);
		if ($ret)
		{
			while($row = $ret->fetchArray(SQLITE3_ASSOC) )
			{
				$res = [$row['gas'], $row['lighting'], $row['tempc']];
				header('Content-Type: application/json');
				echo json_encode($res);
			}
		}
		else
		{
			$res = [0, 0, 0];
			header('Content-Type: application/json');
			echo json_encode($res);
		}
	}
	
	$db->close();
?>