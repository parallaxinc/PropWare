'use strict';

angular.module('propware', [
  'angulartics',
  'angulartics.google.analytics',
  'hljs',
  'ui.bootstrap',
  'propware.reference-nav',
  'propware.about',
  'propware.build-system',
  'propware.cmake-tutorial',
  'propware.contribute',
  'propware.cxx-crash-course',
  'propware.download',
  'propware.getting-started',
  'propware.limitations',
  'propware.related-links',
  'propware.troubleshooting',
  'propware.using-an-ide'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.otherwise(
      {redirectTo: '/about'}
    );
  }
]);

angular.module('propware.reference-nav', []).directive('referenceNav', function () {
  return {
    templateUrl: 'src/reference-nav.html'
  };
});

angular.module('propware.about', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/about', {
      templateUrl: 'src/about.html'
    });
  }
]);

angular.module('propware.getting-started', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/getting-started', {
      templateUrl: 'src/getting-started.html'
    });
  }
]);

angular.module('propware.download', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/download', {
      templateUrl: 'src/download.html'
    });
  }
]);

angular.module('propware.related-links', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/related-links', {
      templateUrl: 'src/related-links.html'
    });
  }
]);

// Reference

angular.module('propware.build-system', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/build-system', {
      templateUrl: 'src/reference/build-system.html'
    });
  }
]);

angular.module('propware.cmake-tutorial', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/cmake-tutorial', {
      templateUrl: 'src/reference/cmake-tutorial.html'
    });
  }
]);

angular.module('propware.contribute', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/contribute', {
      templateUrl: 'src/reference/contribute.html'
    });
  }
]);

angular.module('propware.cxx-crash-course', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/cxx-crash-course', {
      templateUrl: 'src/reference/cxx-crash-course.html'
    });
  }
]);

angular.module('propware.limitations', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/limitations', {
      templateUrl: 'src/reference/limitations.html'
    });
  }
]);

angular.module('propware.troubleshooting', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/troubleshooting', {
      templateUrl: 'src/reference/troubleshooting.html'
    });
  }
]);

angular.module('propware.using-an-ide', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/using-an-ide', {
      templateUrl: 'src/reference/using-an-ide.html'
    });
  }
]);
