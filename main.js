$(function() {
  var r = 0, g = 0, b = 0, r2 = 0, g2 = 0, b2 = 0, d = 0;
  $('.color').change(function(e){
    var t = e.target,
        v = t.value;
    switch(t.id) {
      case 'c1':
        r = hexToRgb(v).r;
        g = hexToRgb(v).g;
        b = hexToRgb(v).b;
        break;
      case 'c2':
        r2 = hexToRgb(v).r;
        g2 = hexToRgb(v).g;
        b2 = hexToRgb(v).b;
        break;
    }
    sendRequest(r,g,b,r2,g2,b2,d);
  });
  $('.range').change(function(e){
    d = e.target.value * 1;
    $(this).attr('data-val', d);
    sendRequest(r,g,b,r2,g2,b2,d);
  });
  function sendRequest(r,g,b,r2,g2,b2,d){
    $.ajax('http://ESP-101/?pin='+r+','+g+','+b+','+r2+','+g2+','+b2+','+d);
  }
  function hexToRgb(hex) {
      var result = /^#?([a-f\d]{2})([a-f\d]{2})([a-f\d]{2})$/i.exec(hex);
      return result ? {
          r: parseInt(result[1], 16),
          g: parseInt(result[2], 16),
          b: parseInt(result[3], 16)
      } : null;
  }
});
