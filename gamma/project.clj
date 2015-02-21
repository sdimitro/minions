(defproject gamma "0.1.0-SNAPSHOT"
  :description "A register machine simulator"
  :url "http://amazingdim1.wordpress.com"
  :license {:name "Eclipse Public License"
            :url "http://www.eclipse.org/legal/epl-v10.html"}
  :dependencies [[org.clojure/clojure "1.6.0"]]
  :target-path "target/%s"
	:repl-options {:init-ns gamma.core}
  :profiles {:dev {:dependencies [[midje "1.6.0" ]]
                  :plugins [[lein-midje "3.1.3"]]}})
