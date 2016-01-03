<?php
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
		$res = array(NULL);
		header('Content-Type: application/json');
		echo json_encode($res);
	} 
	else 
	{
		$db->busyTimeout(500);
		$sql =<<<EOF
		SELECT * FROM sensors where active_state = 1 order by active_time desc;
EOF;
		$ret = $db->query($sql);
		if ($ret)
		{
			$res = array();
			while($row = $ret->fetchArray(SQLITE3_ASSOC) )
			{
				array_push($res, $row['unique_number']);
			}
			header('Content-Type: application/json');
			echo json_encode($res);
		}
		else
		{
			$res = array(NULL);
			header('Content-Type: application/json');
			echo json_encode($res);
		}
	}
	
	$db->close();
?>