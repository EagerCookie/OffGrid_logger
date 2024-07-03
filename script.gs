
var SS = SpreadsheetApp.openById('%INSERT_YOUR_GOOGLE_SHEETS_API_KEY_HERE%');
var str = "";

function addEmptyRows() {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("24-9-2023");
  var numRowsToAdd = 29900; //script receives data every 3 seconds so 86400/3 = 28800 enties per day
  
  // Get the current number of rows in the sheet
  var currentNumRows = sheet.getMaxRows();
  
  // Check if we need to add more rows

    // Calculate how many rows to add
    var numRowsToAdd = numRowsToAdd - currentNumRows;
    
    // Insert empty rows at the bottom of the sheet
    sheet.insertRowsAfter(currentNumRows, numRowsToAdd);

}



function moveChartToOwnSheet() {
  var spreadsheet = SpreadsheetApp.getActiveSpreadsheet();
  var originalSheet = spreadsheet.getActiveSheet();
  
  // Create a new sheet for the chart
  var chartSheet = spreadsheet.insertSheet('Chart Sheet');
  
  // Get the last column in the first row (assuming headers are in the first row)
  var lastColumn = originalSheet.getLastColumn();
  
  // Define the range for the chart data (including headers)
  var chartRange = originalSheet.getRange(1, 1, originalSheet.getLastRow(), lastColumn);
  
  // Create the chart
  var chart = chartSheet.newChart()
    .asColumnChart()
    .addRange(chartRange)
    .setOption('title', 'My Chart Title')
    .setPosition(5, 5, 0, 0)
    .build();
  
  // Remove the chart from the original sheet
  originalSheet.removeChart(chart);
  
  // Adjust the chart's range to exclude headers
  chart = chart.modify()
    .removeRange(originalSheet.getRange(1, 1, 1, lastColumn))
    .setOption('title', 'My Chart Title')
    .build();
  
  // Move the chart to the chart sheet
  chartSheet.insertChart(chart);
}


function hexStringToBytes(hex) {
    const bytes = [];
    for (let i = 0; i < hex.length; i += 2) {
    bytes.push(parseInt(hex.substr(i, 2), 16));
    }
    return bytes;
  }


function createAreaChart() {
  var sheet = SpreadsheetApp.getActiveSpreadsheet().getSheetByName("page1");
  var chart = sheet.newChart()
      .setChartType(Charts.ChartType.AREA)
      .setOption('title', 'Диаграмма с областями')
      .setOption('xAxisTitle', sheet.getRange("A1").getValue()) // Название оси X из ячейки A1
      .setOption('yAxisTitle', 'Значения') // Название оси Y
      .setOption('legend', {position: 'top'})
      //.setOption('isStacked', true)
      //.setOption('series', { 0:{color: 'green', visibleInLegend: true},1:{color: 'yellow', visibleInLegend: true},2:{color: 'blue', visibleInLegend: true},  3:{color: 'red', visibleInLegend: true}})
      //.setDimensions(600, 400) // Размеры диаграммы
      .setPosition(1, 6, 0, 0) // Позиция диаграммы в таблице
      //.setOption('useFirstColumnAsDomain', true);

  // Диапазон данных для столбцов B, C, D и E
  var dataRange = sheet.getRange("A1:E" + sheet.getLastRow());

  // Добавление данных из диапазона на диаграмму
  chart = chart.addRange(dataRange);

  //sheet.insertChart(chart.build(), chartLocation);
  chart = chart.setOption('useFirstColumnAsDomain', true);
  sheet.insertChart(chart.build());
}

function createNewPage(){
  var now = new Date();
  var dateString = now.getDate()+'-'+(now.getMonth()+1)+'-'+now.getFullYear();
  var sheet = SS.getSheetByName(dateString);
  //Logger.log(sheet);
  if(sheet == null){
    sheet = SS.insertSheet(dateString);
    //Logger.log(sheet);
  }
}



//Logger glued both inverter and BMS data into one batch that sholud be separated for following parsing
function parseInverterQPIGS(rawString) {
  //var rawString = '(227.9 50.0 227.9 50.0 1025 0977 018 384 00.00 000 000 0037 0000 235.8 00.00 00000 00010000 00 00 01030 010K“';
  try{
    var values = rawString.split('%20');
  }
  catch(f){
    return ContentService.createTextOutput("Error! RawString empty or in incorrect format.");
  }
  var Power_VA = Number(values[4]), 
      Power_W=Number(values[5]), 
      OutP_percentage = Number(values[6]),
      BusVoltage = Number(values[7]),
      BatteryVoltage = parseFloat(values[8]),
      BattCharge = Number(values[9]),
      BattCapacity = Number(values[10]),
      HeatsikTemp=Number(values[11]), 
      PVBattCharge = Number(values[12]),
      PV_Voltage = parseFloat(values[13]),
      SCC_Bat = parseFloat(values[14]),
      BattDischarge = Number(values[15]),
      PV_W =Number(values[19]);
      
      BMS_data = hexStringToBytes(values[21]);
      BMS_V = (((BMS_data[59])<<8)|(BMS_data[60]))/100;
      BMS_A = (((BMS_data[62]&0x7F)<<8)|(BMS_data[63]))/100;
 

  
  return [Power_VA, Power_W, OutP_percentage, BusVoltage, 
          BatteryVoltage, BattCharge, BattCapacity, HeatsikTemp, 
          PVBattCharge, PV_Voltage, SCC_Bat, BattDischarge, PV_W,
          BMS_V, BMS_A ]
}


function doGet(e) {

  var inputData;




  try { 
    inputData =(e.queryString);
  } 
  catch(f){
    return ContentService.createTextOutput("Error! Request body empty or in incorrect format.");
  }


  var now = new Date();
  var resArray = new Array();


  // Getting time to match with dataValues
  resArray.push(now.getHours()+':'+now.getMinutes()+':'+now.getSeconds());
  var dataArray = resArray.concat(parseInverterQPIGS(inputData));

  // Getting current date to check if it's new day began already
  var dateString = now.getDate()+'-'+(now.getMonth()+1)+'-'+now.getFullYear();
  var sheet = SS.getSheetByName(dateString);

  // Creating new sheet every next day if it dont't exist yet
  if(sheet == null){
    sheet = SS.insertSheet(dateString);
    sheet.appendRow(["Time","Power_VA","Power_W","OutP_percentage","BusVoltage",
                      "BatteryVoltage", "BattCharge", "BattCapacity", "HeatsikTemp", 
                      "PVBattCharge", "PV_Voltage", "SCC_Bat", "BattDischarge", "PV_W",
                      "BMS_V", "BMS_A"
                    ]);

    //script receives data every 3 seconds so 86400/3 = 28800 maximum enties per day
    //empty rows are needed to be included in chart data range to be updatable
    var numRowsToAdd = 29900; 

    // Get the current number of rows in the sheet
    var currentNumRows = sheet.getMaxRows();

    // Check if we need to add more rows
    // Calculate how many rows to add
    var numRowsToAdd = numRowsToAdd - currentNumRows;
    if(numRowsToAdd>0){
    // Insert empty rows at the bottom of the sheet
      sheet.insertRowsAfter(currentNumRows, numRowsToAdd);
    }
  }

  sheet.appendRow(dataArray);
  SpreadsheetApp.flush();
  return 'None';

}











