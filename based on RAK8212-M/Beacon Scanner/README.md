RAK4600 supports scanning Beacon devices.The `rui_ble_scan_adv` function is added to the project, and the scanned broadcast data can be obtained in the parameters of the function. You can filter the target Beacon data according to your needs.

Note:You need to configure Beacon Scanner mode through the AT command `at+set_config=ble:work_mode:2:0`.

