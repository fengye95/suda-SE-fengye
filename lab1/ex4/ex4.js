var codes = [];//取件码数组
var right_codes = "1,2,3,4,5,6" //正确的取件码
var code_inputs = document.getElementsByTagName("input");//取件码输入框
var times = 60
var countdown = document.getElementsByClassName("countdown")[0].firstElementChild;//取件码输入框
// 倒计时
setInterval(() => {
    if (times > 0) {
        countdown.innerText = times;
        times -= 1;
    } else {
        alert("操作超时");
        times = 60;
    }
}, 1000);
// 数据键盘单击事件
function tap(key) {
    if (key == -1) {
        codes.pop();
    } else if (codes.length < 6) {
        codes.push(key);
    }
    refresh_input();
}
// 更新取件输入框数字
function refresh_input() {
    for (var i = 0, len = code_inputs.length; i < len; i++) {
        if (codes[i] != undefined) {
            code_inputs[i].value = codes[i];
        } else {
            code_inputs[i].value = "";
        }
    };
}
function collect() {
    //判断输入长度
    if(codes.length==6){
        //判断输入内容是否正确
        if(codes.toString()==right_codes){
            alert("箱门已打开")
        }else{
            alert("取件码错误")
        }
    }else{
        alert("请输入完整取件码")
    }
}