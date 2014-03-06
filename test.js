$( document ).ready(function() {
    
    $("#post").click(function(){
        var inputvalue = $("#input").val();
        $("#wall").prepend('<p>'+ inputvalue + '<button class ="like">Like</button><br><br></p>');
        $("p").hide(5000);
        d
        
        $(".like").click(function()
        {
             $(this).text("I like this");
        });
 
    });
    
});
