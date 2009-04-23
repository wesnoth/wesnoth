<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<?python import sitetemplate ?>
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="sitetemplate">

<head py:match="item.tag=='{http://www.w3.org/1999/xhtml}head'" py:attrs="item.items()">
    <meta content="text/html; charset=UTF-8" http-equiv="content-type" py:replace="''"/>
    <title py:replace="''">Your title goes here</title>
    <meta py:replace="item[:]" name="description" content="master template"/>
    <style type="text/css" media="screen">
        #pageLogin
        {
            font-size: 10px;
            font-family: verdana;
            text-align: right;
        }
    </style>
    <link rel="stylesheet" type="text/css" media="screen" href="../static/css/style.css"
        py:attrs="href=tg.url('/static/css/style.css')"/>
</head>

<body py:match="item.tag=='{http://www.w3.org/1999/xhtml}body'" py:attrs="item.items()">
    <div py:if="tg.config('identity.on') and not defined('logging_in')" id="pageLogin">
        <span py:if="tg.identity.anonymous">
            <a href="${tg.url(tg.identity.login_url)}">Login</a>
        </span>
        <span py:if="not tg.identity.anonymous">
            Welcome ${tg.identity.user.display_name or tg.identity.user.user_name}.
            <a href="${tg.url('/logout')}">Logout</a>
        </span>
    </div>

    <div id="header">&#160;</div>

    <div id="main_content">
        <div id="status_block" class="flash"
            py:if="value_of('tg_flash', None)" py:content="tg_flash"></div>
        <div py:replace="[item.text]+item[:]">page content</div>
    </div>

    <div id="footer">
        <img src="${tg.url('/static/images/under_the_hood_blue.png')}"
            alt="TurboGears under the hood" />
        <p>TurboGears is a open source front-to-back web development framework
            written in Python</p>
        <p>Copyright &#169; 2007 Kevin Dangoor</p>
    </div>
</body>

</html>
