const char WebPage[] PROGMEM = R"rawliteral(
<HTML>
  <HEAD>
     <TITLE>ESP32 Camera</TITLE>
     <meta http-equiv="refresh">
     <meta charset="utf-8">
      <style>
          input[type="text"] {
              border-color: #FF003C;
              border-width: 3px;
              border-style: solid;
              border-radius: 10px;
              font-size: 25px;
              font-family: monospace;
              font-weight: bold;
              margin-top: -10px;
              color: #FF547C;
              transition: 0.5s;
              opacity: 1;
          }
          input[type="password"] {
              border-color: #FF003C;
              border-width: 3px;
              border-style: solid;
              border-radius: 10px;
              font-size: 25px;
              font-family: monospace;
              font-weight: bold;
              margin-top: -10px;
              color: #FF547C;
              transition: 0.5s;
              opacity: 1;
          }
          input[type="email"] {
              border-color: #FF003C;
              border-width: 3px;
              border-style: solid;
              border-radius: 10px;
              font-size: 25px;
              font-family: monospace;
              font-weight: bold;
              margin-top: -10px;
              color: #FF547C;
              transition: 0.5s;
              opacity: 1;
          }
          input[type="text"]::-webkit-input-placeholder {
              color: #FF003C
          }
          input[type="text"]:focus, input:focus {
              border-color: #FF547C;
              outline: none;
          }
          input[type="email"]::-webkit-input-placeholder {
              color: #FF003C
          }
          input[type="email"]:focus, input:focus {
              border-color: #FF547C;
              outline: none;
          }
          input[type="password"]::-webkit-input-placeholder {
              color: #FF003C
          }
          input:-webkit-autofill { 
              -webkit-box-shadow: 0 0 0 30px white inset !important;
              -webkit-text-fill-color: #FF547C;
          }
          .button {
              background-color: #ffffff;
              border-color: #FF003C;
              border-style: solid;
              border-width: 4px;
              border-radius: 10px;
              font-size: 27px;
              font-family: monospace;
              opacity: 1;
              transition: 0.3s;
              font-weight: bold;
              color: #FF003C;
          }
          .button:hover {
              opacity: 1;
              transition: 0.3s;
              border-color: #FF547C;
          }
          .button:focus {
              background-color: #FF003C;
              color: white;
              outline: none;
          }
      </style>
  </HEAD>
  <BODY>
    <CENTER>
        <h1 style="font-family:Helvetica;font-size:42;color:#FF003C;user-select:none;margin-bottom:6px">ESP32 Camera</h1>
        <form class="form-horizontal">
            <div class="form-group">
                <label class="col-md-4 control-label" for="ssid_name"></label>  
                <div class="col-md-6">
                    <p><input id="ssid_name" name="ssid_name" required type="text" spellcheck="false" placeholder="Tên mạng Wi-Fi"></p>
                </div>
            </div>
            <div class="form-group">
                <label class="col-md-4 control-label" for="ssid_pass"></label>  
                <div class="col-md-6">
                    <p><input id="ssid_pass" name="ssid_pass" required type="password" spellcheck="false" minlength="8" placeholder="Mật khẩu mạng WiFi"></p>
                </div>
            </div>
            <div class="form-group">
                <label class="col-md-4 control-label" for="user_email"></label>  
                <div class="col-md-6">
                    <p><input id="user_email" name="user_email" required type="email" spellcheck="false"  placeholder="Email người dùng"></p>
                </div>
            </div>
            <div class="form-group">
                <label class="col-md-4 control-label" for="user_pass"></label>  
                <div class="col-md-6">
                    <p><input id="user_pass" name="user_pass" required type="password" spellcheck="false" minlength="6" placeholder="Mật khẩu người dùng"></p>
                </div>
            </div>
        <div class="form-group">
            <label class="col-md-4 control-label"></label>
            <div class="col-md-8">
                <button style="margin-top:-6px" required class="button">Lưu các thiết lập</button>
            </div>
        </div>
    </CENTER> 
  </BODY>
</HTML>
)rawliteral";


const char HandleNotFound[] PROGMEM = R"rawliteral(
<HTML>
  <HEAD>
     <TITLE>ESP32 Camera</TITLE>
     <meta charset="utf-8">
  </HEAD>
  <BODY>
    <CENTER>
        <h1 style="font-family:Helvetica;font-size:32;color:#FF003C;user-select:none;margin-bottom:-20px">Đã nhập đối số không hợp lệ!</h1>
        <h1 style="font-family:Helvetica;font-size:30;color:#FF003C;user-select:none">Đối số hợp lệ: /mjpeg</h1>
    </CENTER> 
  </BODY>
</HTML>
)rawliteral";
