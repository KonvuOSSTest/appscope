ARG JDK_IMAGE
FROM $JDK_IMAGE

ARG TOMCAT_MAJOR_VERSION=8
ARG TOMCAT_MINOR_VERSION=8.0.49
ARG KEYTOOL=keytool
ARG DNLD_HEXDUMP

ENV CATALINA_HOME /opt/tomcat
ENV PATH="${PATH}:${CATALINA_HOME}/bin"

ADD https://archive.apache.org/dist/tomcat/tomcat-${TOMCAT_MAJOR_VERSION}/v${TOMCAT_MINOR_VERSION}/bin/apache-tomcat-${TOMCAT_MINOR_VERSION}.tar.gz /apache-tomcat.tar.gz

# for hexdump.  java 11 and under seems to use apt-get, 12 and 13 uses yum, 14 uses microdnf
RUN eval $DNLD_HEXDUMP

RUN tar zxf apache-tomcat.tar.gz && \
    rm apache-tomcat.tar.gz && \
    mv apache-tomcat* $CATALINA_HOME

RUN mkdir /opt/tomcat/certs && \
    mkdir -p /opt/test-runner/logs && \
    mkdir -p /opt/test-runner/bin

RUN ${KEYTOOL} -genkey -alias "tomcat" -dname "CN=scope,O=cribl" -keyalg RSA -storetype pkcs12 -keystore /opt/tomcat/certs/tomcat.p12 -storepass changeit -keypass changeit

COPY ./java/server.xml $CATALINA_HOME/conf/server.xml
COPY ./java/test-ssl.sh /opt/test-runner/bin/test-ssl.sh
RUN chmod +x /opt/test-runner/bin/test-ssl.sh

RUN mkdir -p /opt/javassl
COPY ./java/SSLSocketClient.java /opt/javassl/SSLSocketClient.java
RUN javac -d /opt/javassl/ /opt/javassl/SSLSocketClient.java

ENV SCOPE_LOG_LEVEL=info
ENV SCOPE_METRIC_VERBOSITY=4
ENV SCOPE_EVENT_LOGFILE=false
ENV SCOPE_EVENT_CONSOLE=false
ENV SCOPE_EVENT_METRIC=false
ENV SCOPE_EVENT_HTTP=true
ENV SCOPE_SUMMARY_PERIOD=1
ENV SCOPE_EVENT_DEST=file:///opt/test-runner/logs/events.log
ENV SCOPE_METRIC_DEST=udp://localhost:8125
ENV SCOPE_LOG_DEST=file:///opt/test-runner/logs/scope.log

ENV PATH="/usr/local/scope:/usr/local/scope/bin:${PATH}"
RUN echo "export PATH=${PATH}" >> /etc/profile.d/path.sh
COPY scope-profile.sh /etc/profile.d/scope.sh
COPY gdbinit /root/.gdbinit

RUN  mkdir /usr/local/scope && \
     mkdir /usr/local/scope/bin && \
     mkdir /usr/local/scope/lib && \
     ln -s /opt/appscope/bin/linux/$(uname -m)/scope /usr/local/scope/bin/scope && \
     ln -s /opt/appscope/bin/linux/$(uname -m)/ldscope /usr/local/scope/bin/ldscope && \
     ln -s /opt/appscope/lib/linux/$(uname -m)/libscope.so /usr/local/scope/lib/libscope.so

COPY java/scope-test /usr/local/scope/scope-test

COPY docker-entrypoint.sh /
ENTRYPOINT ["/docker-entrypoint.sh"]
CMD ["test"]

