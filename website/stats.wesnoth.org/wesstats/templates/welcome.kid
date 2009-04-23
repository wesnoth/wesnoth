<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xmlns:py="http://purl.org/kid/ns#"
    py:extends="'master.kid'">
<head>
<meta content="text/html; charset=utf-8" http-equiv="Content-Type" py:replace="''"/>
<title>Welcome to TurboGears</title>
</head>
<body>

  <div id="sidebar">
    <h2>Learn more</h2>
    Learn more about TurboGears and take part in its
    development
    <ul class="links">
      <li><a href="http://www.turbogears.org">Official website</a></li>
      <li><a href="http://docs.turbogears.org">Documentation</a></li>
      <li><a href="http://trac.turbogears.org/turbogears/">Trac
        (bugs/suggestions)</a></li>
      <li><a href="http://groups.google.com/group/turbogears"> Mailing list</a> </li>
    </ul>
    <span py:replace="now">now</span>
  </div>
  <div id="getting_started">
    <ol id="getting_started_steps">
      <li class="getting_started">
        <h3>Model</h3>
        <p> <a href="http://docs.turbogears.org/1.0/GettingStarted/DefineDatabase">Design models</a> in the <span class="code">model.py</span>.<br/>
          Edit <span class="code">dev.cfg</span> to <a href="http://docs.turbogears.org/1.0/GettingStarted/UseDatabase">use a different backend</a>, or start with a pre-configured SQLite database. <br/>
          Use script <span class="code">tg-admin sql create</span> to create the database tables.</p>
      </li>
      <li class="getting_started">
        <h3>View</h3>
        <p> Edit <a href="http://docs.turbogears.org/1.0/GettingStarted/Kid">html-like templates</a> in the <span class="code">/templates</span> folder;<br/>
        Put all <a href="http://docs.turbogears.org/1.0/StaticFiles">static contents</a> in the <span class="code">/static</span> folder. </p>
      </li>
      <li class="getting_started">
        <h3>Controller</h3>
        <p> Edit <span class="code"> controllers.py</span> and <a href="http://docs.turbogears.org/1.0/GettingStarted/CherryPy">build your
          website structure</a> with the simplicity of Python objects. <br/>
          TurboGears will automatically reload itself when you modify your project. </p>
      </li>
    </ol>
    <div class="notice"> If you create something cool, please <a href="http://groups.google.com/group/turbogears">let people know</a>, and consider contributing something back to the <a href="http://groups.google.com/group/turbogears">community</a>.</div>
  </div>
  <!-- End of getting_started -->
</body>
</html>
