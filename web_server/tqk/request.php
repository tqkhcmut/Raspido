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
		$res = array($GLOBALS['gas'], $GLOBALS['lighting'], $GLOBALS['tempc']);
		header('Content-Type: application/json');
		echo json_encode($res);
	} 
	else 
	{
		$db->busyTimeout(500);
		$sql =<<<EOF
		SELECT * FROM sensor_values where unique_number == $unique_num order by record_count desc limit 1;
EOF;
		$ret = $db->query($sql);
		if ($ret)
		{
			while($row = $ret->fetchArray(SQLITE3_ASSOC) )
			{
				$res = array($row['gas'], $row['lighting'], $row['tempc']);
				$GLOBALS['gas'] = $row['gas'];
				$GLOBALS['lighting'] = $row['lighting'];
				$GLOBALS['tempc'] = $row['tempc'];
				header('Content-Type: application/json');
				echo json_encode($res);
			}
		}
		else
		{
			$res = array($GLOBALS['gas'], $GLOBALS['lighting'], $GLOBALS['tempc']);
			header('Content-Type: application/json');
			echo json_encode($res);
		}
	}
	
	$db->close();
?>