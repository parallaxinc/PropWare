'use strict';

angular.module('propware', [
  'angulartics',
  'angulartics.google.analytics',
  'hljs',
  'ui.bootstrap',
  'propware.about',
  'propware.cmake-reference',
  'propware.cmake-overview',
  'propware.contribute',
  'propware.cxx-crash-course',
  'propware.download',
  'propware.getting-started',
  'propware.limitations',
  'propware.cmake-by-example',
  'propware.related-links',
  'propware.troubleshooting',
  'propware.build-from-source',
  'propware.using-an-ide'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.otherwise(
      {redirectTo: '/about'}
    );
  }
]);

angular.module('propware').constant('REFERENCE_PAGES', {
  '#/reference/cmake-overview': 'CMake Overview',
  '#/reference/cmake-by-example': 'CMake by Example',
  '#/reference/cmake-reference': 'CMake Reference',
  '#/reference/using-an-ide': 'Using an IDE',
  '#/reference/cxx-crash-course': 'C++ Crash Course',
  '#/reference/limitations': 'Limitations',
  '#/reference/build-from-source': 'Build from Source',
  '#/reference/troubleshooting': 'Troubleshooting',
  '#/reference/contribute': 'Contribute'
});

angular.module('propware').directive('referenceNav', function () {
    return {
      templateUrl: 'src/reference/nav.html'
    };
  });

angular.module('propware').run(function ($rootScope, $location, $anchorScroll, REFERENCE_PAGES) {
  //when the route is changed scroll to the proper element.
  $rootScope.$on('$routeChangeSuccess', function () {
    if ($location.hash()) {
      $anchorScroll();
    }
  });

  // Provide the list of reference pages to all HTML pages
  $rootScope.REFERENCE_PAGES = REFERENCE_PAGES;
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

angular.module('propware.cmake-reference', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/cmake-reference', {
      templateUrl: 'src/reference/cmake-reference.html'
    });
  }
]);

angular.module('propware.cmake-overview', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/cmake-overview', {
      templateUrl: 'src/reference/cmake-overview.html'
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

angular.module('propware.cmake-by-example', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/cmake-by-example', {
      templateUrl: 'src/reference/cmake-by-example.html'
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

angular.module('propware.build-from-source', [
  'ngRoute'
]).config([
  '$routeProvider',
  function ($routeProvider) {
    $routeProvider.when('/reference/build-from-source', {
      templateUrl: 'src/reference/build-from-source.html'
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
