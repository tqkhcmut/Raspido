<?php
	class MyDB extends SQLite3
	{
		function __construct()
		{
			$this->open('/var/www/tqk/thesis.db');
		}
	}
	$db = new MyDB();
	if(!$db)
	{
		$res = array('0.0', '0.0', '0.0');
		header('Content-type: application/json');
		echo json_encode($res);
	} 
	else 
	{
		$db->busyTimeout(500);
		$sql =<<<EOF
		SELECT * FROM sensor_values ORDER BY time_record DESC LIMIT 1;
EOF;
		$ret = $db->query($sql);
		if ($ret)
		{
			while($row = $ret->fetchArray(SQLITE3_ASSOC) ){
				$res = array($row['gas'], $row['lighting'], $row['tempc']);
				header('Content-type: application/json');
				echo json_encode($res);
			}
		}
		else
		{
			$res = array('0.0', '0.0', '0.0');
			header('Content-type: application/json');
			echo json_encode($res);
		}
	}
	
	$db->close();
	
?>	