<html>

<head>
  <meta http-equiv="content-type" content="text/html;charset=utf-8">

  <style TYPE="text/css"><!--

  body {
    background-color: #515048;
    font-family: Courier;
  }


  #message {
    height: 73px;
    width: 80%;
  	background-color: #6f6f6f;
  	border: none;
  	color: #ffbb00;
    font-family: Courier;
    font-size: 42px;
  }

  #images {
    padding: 10px;
    margin-right: 10px;
    float: right;
  }

  #messagePanel {
    padding: 10px;
  }

  .panelElement {
    margin-left: auto;
    margin-right: auto;
    padding: 10px;
  }

  .hot {
    color: #ff0000
  }


  #messageSend {
    background-color: #ffbb00;
    width: 200px;
    font-family: Courier;
    font-size: 21;
  }

  ::-webkit-input-placeholder { /* WebKit browsers */
      color: #5f5f5f;
  }
  :-moz-placeholder { /* Mozilla Firefox 4 to 18 */
      color: black;
  }
  ::-moz-placeholder { /* Mozilla Firefox 19+ */
      color: black;
  }
  :-ms-input-placeholder { /* Internet Explorer 10+ */
      color: black;
  }

  --></style>

</head>

<body>

<?

  // Configuration: set you spark core ID and access token here
  $spark_id = 'xxxxxxxxxxxxxxxxxxxxxxxx';
  $spark_access_token = 'yyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyyy';

  $msg = '';

  if (isset($_REQUEST['message'])) {
    $postfields =
      'access_token=' . $spark_access_token .
      '&args=' . urlencode($_REQUEST['message']);
    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, 'https://api.spark.io/v1/devices/' . $spark_id . '/message');
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_FOLLOWLOCATION, true);
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, $postfields);
    curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, 0);
    $result = curl_exec($ch);
    $answer = json_decode($result, true);
    if ($answer['return_value']==1) {
      $msg = 'delivered!';
    }
    else {
      $msg = 'Error, message not delivered (MessageTorch might not be running)';
    }
  }

?>



<div id="images">
  <img src="MessageTorch_spark_message.gif"/>
</div>

<div id="messagePanel">
  <div class="panelElement">
    <h1>MessageTorch</h1>
    <p>Message will appear on our WS2812 LED / Spark.io core based "torch" when it is running. Just try :-)</p>

    <p>If you want to build a MessageTorch yourselves: it's open source, grab it from
    <a target="_blank" href="https://github.com/plan44/messagetorch">github</a></p>

  </div>

  <form action="<?= $_SERVER['PHP_SELF']; ?>">
    <div class="panelElement">
      <input id="message"  name="message" type="text" placeholder="type message here"/>
    </div>
    <? if (strlen($msg)>0) { ?>
    <div class="panelElement">
      <p class="hot"><?= $msg; ?></p>
    </div>
    <? } ?>
    <div class="panelElement">
      <button id="messageSend" type="submit">Send</button>
    </div>
  </form>

</div>

</body>

</html>
