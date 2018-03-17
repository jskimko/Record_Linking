package com.jsk.RecordLinker;

import javafx.application.Application;
import javafx.event.ActionEvent;
import javafx.event.EventHandler;
import javafx.geometry.Insets;
import javafx.geometry.Pos;
import javafx.scene.Scene;
import javafx.scene.control.*;
import javafx.scene.layout.*;
import javafx.scene.text.Text;
import javafx.stage.FileChooser;
import javafx.stage.Stage;

import java.io.File;
import java.io.IOException;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.List;

public class RecordLinker extends Application {

    private static final String TITLE = "Record Linker";
    private static final int NUM_FILES_FIELDS = 5;
    private static final int NUM_PARAM_FIELDS = 8;

    private Label[] fileLabels;
    private Text[] fileTexts;
    private Button[] fileButtons;
    private Label[] paramLabels;
    private TextField[] paramTexts;
    private Button resetBtn;
    private Button saveBtn;
    private Button loadBtn;
    private Button runBtn;

    private TextArea logText;
    private TabPane tabPane;
    private Tab configTab, logTab;


    public static void main(String[] args) {
        launch(args);
    }

    @Override
    public void start(Stage stage) {
        stage.setTitle(TITLE);

        GridPane grid = new GridPane();
        grid.setAlignment(Pos.TOP_LEFT);
        grid.setHgap(10);
        grid.setVgap(10);
        grid.setPadding(new Insets(25, 25, 25, 25));
        HBox.setHgrow(grid, Priority.ALWAYS);
        VBox.setVgrow(grid, Priority.ALWAYS);

        fileLabels = new Label[NUM_FILES_FIELDS];
        fileLabels[0] = new Label("Data 1:");
        fileLabels[1] = new Label("Data 2:");
        fileLabels[2] = new Label("Names 1:");
        fileLabels[3] = new Label("Names 2:");
        fileLabels[4] = new Label("Std_Names:");
        for (int i = 0; i< fileLabels.length; i++) {
            grid.add(fileLabels[i], 0, i);
        }

        fileTexts = new Text[NUM_FILES_FIELDS];
        for (int i = 0; i< fileTexts.length; i++) {
            fileTexts[i] = new Text();
            grid.add(fileTexts[i], 1, i);
        }

        fileButtons = new Button[NUM_FILES_FIELDS];
        for (int i = 0; i< fileButtons.length; i++) {
            final int j = i;
            fileButtons[j] = new Button("...");
            fileButtons[j].setOnAction(new EventHandler<ActionEvent>() {
                @Override
                public void handle(ActionEvent actionEvent) {
                    FileChooser fileChooser = new FileChooser();
                    File file = fileChooser.showOpenDialog(stage);
                    if (file != null) {
                        fileTexts[j].setText(file.getAbsolutePath());
                    }
                }
            });
            grid.add(fileButtons[j], 2, i);
        }

        paramLabels = new Label[NUM_PARAM_FIELDS];
        paramLabels[0] = new Label("Output:");
        paramLabels[1] = new Label("Year 1:");
        paramLabels[2] = new Label("Year 2:");
        paramLabels[3] = new Label("Sex:");
        paramLabels[4] = new Label("Min Age 1:");
        paramLabels[5] = new Label("Max Age 1:");
        paramLabels[6] = new Label("Min Age 2:");
        paramLabels[7] = new Label("Max Age 2:");
        for (int i=0 ;i<paramLabels.length; i++) {
            grid.add(paramLabels[i], 0, i + NUM_FILES_FIELDS);
        }

        paramTexts = new TextField[NUM_PARAM_FIELDS];
        for (int i=0; i<paramTexts.length; i++) {
            paramTexts[i] = new TextField();
            grid.add(paramTexts[i], 1, i + NUM_FILES_FIELDS);
        }

        saveBtn = new Button("Save");
        saveBtn.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                saveFields();
            }
        });
        grid.add(saveBtn, 0, NUM_FILES_FIELDS + NUM_PARAM_FIELDS);

        loadBtn = new Button("Load");
        loadBtn.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                FileChooser fileChooser = new FileChooser();
                File file = fileChooser.showOpenDialog(stage);
                if (file != null) {
                    loadFromFile(file);
                }
            }
        });
        grid.add(loadBtn, 1, NUM_FILES_FIELDS + NUM_PARAM_FIELDS);


        resetBtn = new Button("Reset");
        resetBtn.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                resetFields();
            }
        });
        grid.add(resetBtn, 0, NUM_FILES_FIELDS + NUM_PARAM_FIELDS + 1);

        runBtn = new Button("Run");
        runBtn.setOnAction(new EventHandler<ActionEvent>() {
            @Override
            public void handle(ActionEvent actionEvent) {
                run();
            }
        });
        grid.add(runBtn, 1, NUM_FILES_FIELDS + NUM_PARAM_FIELDS + 1);

        ColumnConstraints constraint1 = new ColumnConstraints();
        constraint1.setPercentWidth(15);
        ColumnConstraints constraint2 = new ColumnConstraints();
        constraint2.setPercentWidth(80);
        ColumnConstraints constraint3 = new ColumnConstraints();
        constraint3.setPercentWidth(5);
        grid.getColumnConstraints().addAll(constraint1, constraint2, constraint3);

        logText = new TextArea();
        logText.setEditable(false);
        logText.setMaxSize(Double.MAX_VALUE, Double.MAX_VALUE);

        tabPane = new TabPane();
        configTab = new Tab();
        configTab.setText("Config");
        configTab.setContent(grid);
        configTab.setClosable(false);

        logTab = new Tab();
        logTab.setText("Log");
        logTab.setContent(logText);
        logTab.setClosable(false);

        tabPane.getTabs().addAll(configTab, logTab);
        Scene scene = new Scene(tabPane, 800, 600);
        stage.setScene(scene);
        stage.show();
    }

    private void run() {
        tabPane.getSelectionModel().select(logTab);
    }

    private void saveFields() {
        SimpleDateFormat format = new SimpleDateFormat("MM-dd-yyyy-hh:mm:ss");
        String name = format.format(new Date()) + ".config";

        StringBuilder sb = new StringBuilder();
        for (Text fileText : fileTexts) {
            sb.append(fileText.getText()).append(System.lineSeparator());
        }
        for (TextField paramText : paramTexts) {
            sb.append(paramText.getText()).append(System.lineSeparator());
        }

        try {
            Files.write(Paths.get(name), sb.toString().getBytes());
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    private void loadFromFile(File file) {
        List<String> lines;
        try {
            lines = Files.readAllLines(file.toPath());
        } catch (IOException e) {
            e.printStackTrace();
            return;
        }

        int i=0;
        for (Text fileText : fileTexts) {
            fileText.setText(lines.get(i++));
        }
        for (TextField paramText : paramTexts) {
            paramText.setText(lines.get(i++));
        }
    }

    private void resetFields() {
        for (Text fileText : fileTexts) {
            fileText.setText("");
        }
        for (TextField paramText : paramTexts) {
            paramText.setText("");
        }
    }

}
